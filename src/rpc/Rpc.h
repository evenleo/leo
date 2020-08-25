
#ifndef __RPC_H__
#define __RPC_H__

#include <iostream>
#include <map>
#include <functional>
#include <tuple>
#include <memory>
#include "serializer.h"
#include "TcpServer.h"
#include "TcpClient.h"
#include "Scheduler.h"

namespace leo {

template<typename T>
struct type_xx { typedef T type; };

template<>
struct type_xx<void> { typedef int8_t type; };

// 打包帮助模板
template<typename Tuple, std::size_t... Index>
void package_Args_impl(Serializer& sr, const Tuple& t, std::index_sequence<Index...>)
{
	((sr << std::get<Index>(t)), ...);
}

template<typename... Args>
void package_Args(Serializer& sr, const std::tuple<Args...>& t)
{
	package_Args_impl(sr, t, std::index_sequence_for<Args...>{});
}

// 用tuple做参数调用函数模板类
template<typename Function, typename Tuple, std::size_t... Index>
decltype(auto) invoke_impl(Function&& func, Tuple&& t, std::index_sequence<Index...>)
{
	return func(std::get<Index>(std::forward<Tuple>(t))...);
}

template<typename Function, typename Tuple>
decltype(auto) invoke(Function&& func, Tuple&& t)
{
	constexpr auto size = std::tuple_size<typename std::decay<Tuple>::type>::value;
	return invoke_impl(std::forward<Function>(func), std::forward<Tuple>(t), std::make_index_sequence<size>{});
}

template<typename R, typename F, typename ArgsTuple>
typename std::enable_if<!std::is_same<R, void>::value, typename type_xx<R>::type >::type
call_helper(F f, ArgsTuple args) 
{
	return invoke(f, args);
}

enum ErrCode {
	RPC_ERR_SUCCESS = 0,
	RPC_ERR_FUNCTION_NOT_REGUIST,
	RPC_ERR_RECV_TIMEOUT
};

template <typename T>
class response_t {
public:
	response_t() : code_(RPC_ERR_SUCCESS) { msg_.clear(); }
	int error_code() const { return code_; }
	std::string error_msg() const { return msg_; }
	T val() const { return val_; }

	void set_val(const T& val) { val_ = val; }
	void set_code(int code) { code_ = code; }
	void set_msg(const std::string& msg) { msg_ = msg; }

	friend Serializer& operator >> (Serializer& in, response_t<T>& d)
	{
		in >> d.code_ >> d.msg_;
		if (d.code_ == 0)
			in >> d.val_;
		return in;
	}
	friend Serializer& operator << (Serializer& out, response_t<T>& d)
	{
		out << d.code_ << d.msg_ << d.val_;
		return out;
	}
private:
	ErrCode code_;
	std::string msg_;
	T val_;
};


class RpcServer {
public:
	RpcServer(int port, int threads) {
		IpAddress addr(port);
		scheduler = std::make_shared<Scheduler>(threads);
		scheduler->startAsync();
		server = std::make_shared<TcpServer>(addr, scheduler);
		server->setConnectionHandler(std::bind(handleClient, this, std::placeholders::_1));
	}
	~RpcServer()
	{
		scheduler->stop();
	}

	void handleClient(TcpConnection::ptr conn) {
		conn->setTcpNoDelay(true);
		Buffer::ptr buffer = std::make_shared<Buffer>();
		while (conn->read(buffer) > 0) {
			std::string str(buffer->peek(), buffer->readableBytes());
			std::cout << "recv: " << str << std::endl;
			conn->write(buffer);
		}
		conn->close();
	}

	void run()
	{
		server->start();
		scheduler->wait();
	}

	std::shared_ptr<Serializer> call_(const std::string& name, const char* data, int len)
	{
		std::shared_ptr<Serializer> sp = std::make_shared<Serializer>();
		if (mapFunctions_.find(name) == mapFunctions_.end())
		{
			*sp.get() << static_cast<int>(RPC_ERR_FUNCTION_NOT_REGUIST);
			*sp.get() << std::string("function not bind: " + name);
			return sp;
		}
		auto func = mapFunctions_[name];
		func(sp.get(), data, len);
		sp.get()->reset();
		return sp;
	}

	template <typename R, typename... Args>
	response_t<R> call(const std::string& name, Args... args)
	{
		using args_type = std::tuple<typename std::decay<Args>::type...>;
		args_type as = std::make_tuple(args...);
		Serializer sr;
		sr << name;
		package_Args(sr, as);
		return net_call<R>(sr);
	}

    template <typename F>
    void regist(const std::string& name, F func)
    {
        mapFunctions_[name] = std::bind(&rpc::callproxy<F>, this, func, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }

	template<typename F, typename S>
    void regist(const std::string& name, F func, S* s)
    {
        mapFunctions_[name] = std::bind(&rpc::callproxy<F, S>, this, func, s, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }

    template<typename F>
    void callproxy(F func, Serializer* pr, const char* data, int len )
    {
        callproxy_(func, pr, data, len);
    }
	template<typename F, typename S>
	void callproxy(F func, S * s, Serializer * pr, const char * data, int len)
	{
		callproxy_(func, s, pr, data, len);
	}

    // 函数指针
	template<typename R, typename... Args>
	void callproxy_(R(*func)(Args...), Serializer* pr, const char* data, int len) {
		callproxy_(std::function<R(Args...)>(func), pr, data, len);
	}

    template<typename R, typename C, typename S, typename... Args>
	void callproxy_(R(C::* func)(Args...), S* s, Serializer* pr, const char* data, int len) {

		using args_type = std::tuple<typename std::decay<Args>::type...>;

		Serializer sr(StreamBuffer(data, len));
		args_type as = sr.get_tuple<args_type>(std::index_sequence_for<Args...>{});

		auto ff = [=](Args... args)->R {
			return (s->*func)(args...);
		};
		typename type_xx<R>::type r = call_helper<R>(ff, as);

		response_t<R> response;
		response.set_code(RPC_ERR_SUCCESS);
		response.set_val(r);
		(*pr) << response;
	}

    // functional
	template<typename R, typename... Args>
	void callproxy_(std::function<R(Args... args)> func, Serializer* pr, const char* data, int len) 
	{
		using args_type = std::tuple<typename std::decay<Args>::type...>;

		Serializer sr(StreamBuffer(data, len));
		args_type as = sr.get_tuple<args_type> (std::index_sequence_for<Args...>{});

		typename type_xx<R>::type r = call_helper<R>(func, as);

		response_t<R> response;
		response.set_code(RPC_ERR_SUCCESS);
		response.set_val(r);
		(*pr) << response;
	}

	template<typename R>
	response_t<R> net_call(Serializer& sr)
	{
		zmq::message_t request(sr.size() + 1);
		memcpy(request.data(), sr.data(), sr.size());
		send(request);
		zmq::message_t reply;
		recv(reply);
		response_t<R> val;
		if (reply.size() == 0) 
		{
			val.set_code(RPC_ERR_RECV_TIMEOUT);
			val.set_msg("recv timeout");
			return val;
		}
		sr.clear();
		sr.input((char *)reply.data(), reply.size());
		sr >> val;
		return val;
	}

private:
	
    std::map<std::string, std::function<void(Serializer*, const char*, int)>> mapFunctions_;
	Scheduler::ptr scheduler;
	TcpServer::ptr server;
};

}

#endif
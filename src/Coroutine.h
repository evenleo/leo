/**
 * 协程，可在当前协程和主协程间切换
 */ 
#ifndef _LEO_COROUTINE_H_
#define _LEO_COROUTINE_H_

#include "Noncopyable.h"

#include <functional>
#include <memory>
#include <stdint.h>
#include <ucontext.h>

namespace leo {

enum class CoroutineState {
	RUNNABLE,		// 可运行，包括初始化，从poll()中返回，从wait()从返回
	BLOCKED,		// 等待epoll中
	TERM,		    //运行结束
};

const uint32_t kStackSize = 1024 * 512;

class Coroutine : public Noncopyable, public std::enable_shared_from_this<Coroutine> {
public:
	typedef std::function<void()> Func;
	typedef std::shared_ptr<Coroutine> ptr;

	Coroutine(Func cb, const std::string& name = "anonymous", uint32_t stack_size = kStackSize);

	~Coroutine();

	static void SwapOut();  // 切换出当前协程

	void swapIn();          // 执行该协程

	Coroutine::Func getCallback() const { return cb_; }

	std::string name() const { return name_; }

	void setState(CoroutineState state) { state_ = state; }

	CoroutineState getState() const { return state_; }

	static uint64_t GetCid();

	static Coroutine::ptr& GetCurrentCoroutine();

private:
	Coroutine();
	static Coroutine::ptr GetMainCoroutine();

	static void RunInCoroutine();

private:
	uint64_t c_id_;         // 协程id
	std::string name_;      // 协程名称
	Func cb_;               // 协程回调
	void* stack_;           // 栈空间
	uint32_t stack_size_;   // 栈大小
	ucontext_t context_;    // 上下文
	CoroutineState state_;  // 协程状态
};

}

#endif

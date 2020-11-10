#ifndef _ARGS_H_
#define _ARGS_H_

#include "rpc/serializer.h"

using namespace leo;

struct RequestVoteArgs
{
    RequestVoteArgs() {}
    RequestVoteArgs(int term, int id, int last_index, int last_term)
        : term_(term), id_(id), last_index_(last_index), last_term_(last_term) {}
    int term_;
    int id_;
    int last_index_;
    int last_term_;

    friend Serializer& operator>>(Serializer& in, RequestVoteArgs& args)
    {
        in >> args.term_ >> args.id_ >> args.last_index_ >> args.last_term_;
        return in;
    }
    friend Serializer& operator<<(Serializer& out, RequestVoteArgs& args)
    {
        out << args.term_ << args.id_ << args.last_index_ << args.last_term_;
        return out;
    }
};

struct RequestVoteReply
{
    RequestVoteReply() {}
    int term_;
    int granted_;

    friend Serializer& operator>>(Serializer& in, RequestVoteReply& args)
    {
        in >> args.term_ >> args.granted_;
        return in;
    }
    friend Serializer& operator<<(Serializer& out, RequestVoteReply& args)
    {
        out << args.term_ << args.granted_;
        return out;
    }
};

#endif
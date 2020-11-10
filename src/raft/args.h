#ifndef _ARGS_H_
#define _ARGS_H_

#include "rpc/serializer.h"

using namespace leo;

struct RequestVoteArgs
{
    RequestVoteArgs() {}
    RequestVoteArgs(uint32_t term, uint32_t id, uint32_t last_index, uint32_t last_term)
        : term_(term), id_(id), last_index_(last_index), last_term_(last_term) {}
    uint32_t term_;
    int32_t id_;
    int32_t last_index_;
    uint32_t last_term_;

    friend Serializer &operator>>(Serializer &in, RequestVoteArgs &args)
    {
        in >> args.term_ >> args.id_ >> args.last_index_ >> args.last_term_;
        return in;
    }
    friend Serializer &operator<<(Serializer &out, RequestVoteArgs &args)
    {
        out << args.term_ << args.id_ << args.last_index_ << args.last_term_;
        return out;
    }
};

struct RequestVoteReply
{
    RequestVoteReply() {}
    uint32_t term_;
    uint32_t granted_;

    friend Serializer &operator>>(Serializer &in, RequestVoteReply &args)
    {
        in >> args.term_ >> args.granted_;
        return in;
    }
    friend Serializer &operator<<(Serializer &out, RequestVoteReply &args)
    {
        out << args.term_ << args.granted_;
        return out;
    }
};

struct AppendEntryArgs
{
    AppendEntryArgs() {}

    AppendEntryArgs(uint32_t term, int32_t id, uint32_t prev_term, int32_t prev_index,
                    int32_t leader_commit) : term_(term), id_(id), prev_term_(prev_term),
                                             prev_index_(prev_index), leader_commit_(leader_commit) {}
    uint32_t term_;
    int32_t id_;
    uint32_t prev_term_;
    int32_t prev_index_;
    int32_t leader_commit_;
    // std::vector</*Mushroom*/ Log> entries_;

    friend Serializer &operator>>(Serializer &in, AppendEntryArgs &args)
    {
        in >> args.term_ >> args.id_ >> args.prev_term_ >> args.leader_commit_;
        return in;
    }
    friend Serializer &operator<<(Serializer &out, AppendEntryArgs &args)
    {
        out << args.term_ << args.id_ << args.prev_term_ << args.leader_commit_;
        return out;
    }
};

struct AppendEntryReply
{
    AppendEntryReply() {}
    uint32_t term_;
    int32_t idx_;

    friend Serializer &operator>>(Serializer &in, AppendEntryReply &args)
    {
        in >> args.term_ >> args.idx_;
        return in;
    }
    friend Serializer &operator<<(Serializer &out, AppendEntryReply &args)
    {
        out << args.term_ << args.idx_;
        return out;
    }
};

#endif
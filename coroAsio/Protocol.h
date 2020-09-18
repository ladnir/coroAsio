#pragma once
#include <iostream>
#include <vector>
#include "cppcoro/task.hpp"
#include "cppcoro/generator.hpp"
#include "cppcoro/async_generator.hpp"
#include "cryptoTools/Network/Channel.h"
#include "cryptoTools/Network/IOService.h"
namespace stde = std::experimental;

inline void panic(std::string msg)
{
    std::cout << msg << std::endl;
    std::terminate();
}

using Channel = oc::Channel;


struct NetBufferBase
{
    virtual int numBuffs() = 0;
    virtual oc::span<oc::u8> getBuff() = 0;
};

struct NetBuffer
{
    NetBuffer() = default;
    NetBuffer(NetBuffer&&) = default;
    NetBuffer(std::unique_ptr<NetBufferBase>&& p) : mBase(std::move(p)) {}

    std::unique_ptr<NetBufferBase> mBase;

    int numBuffs() { if (mBase) return mBase->numBuffs(); return 0; };
    oc::span<oc::u8> getBuff() { if (mBase) return mBase->getBuff(); return {}; };

};

std::unique_ptr<NetBufferBase> makeNetBuff(std::vector<char>& b)
{
    struct vectorNetBuff : public NetBufferBase
    {
        std::vector<char>& mBuff;
        int numBuffs() override { return 1; }
        oc::span<oc::u8> getBuff() {
            return oc::span<oc::u8>((oc::u8*)mBuff.data(), mBuff.size());
        }
    };
}

struct EcSend
{

};

struct Send
{
    NetBuffer mBuff;

    Send(std::vector<char>& b) : mBuff(makeNetBuff(b)) {};
    EcSend getErrorCode();
};

struct PBSend
{
    std::vector<std::vector<char>>& mBuff;

    PBSend(std::vector<std::vector<char>>& b) : mBuff(b) {};
    EcSend getErrorCode();
};


struct EcRecv
{
};

struct Recv
{
    std::vector<char>& mBuff;
    Recv(std::vector<char>& b) : mBuff(b) {};
    EcRecv getErrorCode();
};
struct Initialize
{
    Initialize(std::string name = "") {}
};

struct PartyIdx
{

};

struct ProtocolBuffer
{
    std::vector<std::vector<char>> mBuff;
};


enum ProtoCodes {
    success,
    EndOfRound,
    BadRecv,
    BadSend,
    PendingRecv,
    PendingSend
};

enum ProtoCatagory
{
    // success = 0,
    Suspend = 1,
    Failure
};

std::error_code make_error_code(const ProtoCodes& e);
std::error_condition make_error_condition(const ProtoCatagory& e);
namespace std {
    template <>
    struct is_error_code_enum<ProtoCodes> : true_type {};
    template <>
    struct is_error_condition_enum<ProtoCatagory>
        : true_type {};
}

struct SendAwaiter;
struct RecvAwaiter;
struct RoundAwaiter;

struct EcRound;
class Protocol;
struct ProtocolAwaiter;
struct Round;


struct protocol_promise
{
    using coro_handle = std::experimental::coroutine_handle<protocol_promise>;

    Channel* mChl;
    std::error_code mEC;
    std::exception_ptr mEx;
    ProtocolBuffer* mProtoBuff = nullptr;

    bool hasError();

    void propagateErrors(protocol_promise& child);

    auto get_return_object();
    auto initial_suspend();
    auto final_suspend() noexcept;

    auto return_value(std::error_code ec);

    void unhandled_exception();

    Channel& getChannel() { return*mChl; }

    SendAwaiter await_transform(Send&& res) noexcept;
    RecvAwaiter await_transform(Recv&& res) noexcept;

    //struct EcSendAwaiter
    //{
    //    EcSendAwaiter(EcSend&&);
    //    std::error_code await_resume() noexcept;
    //    bool await_ready() noexcept;
    //    void await_suspend(coro_handle const& handle) noexcept;
    //};
    //auto await_transform(EcSend&& res) noexcept {
    //    return EcSendAwaiter(std::move(res));
    //}

    //struct EcRecvAwaiter
    //{
    //    EcRecvAwaiter(EcRecv&&);

    //    std::error_code await_resume() noexcept;
    //    bool await_ready() noexcept;
    //    void await_suspend(coro_handle const& handle) noexcept;
    //};
    //auto await_transform(EcRecv&& res) noexcept {
    //    return EcRecvAwaiter(std::move(res));
    //}


    struct InitAwaiter
    {
        InitAwaiter(Initialize&&) {}
        SID await_resume() noexcept { return {}; }
        bool await_ready() noexcept { return true; }
        void await_suspend(coro_handle const& handle) noexcept {}
    };
    auto await_transform(Initialize&& res) noexcept {
        return InitAwaiter(std::move(res));
    }

    struct EoRAwaiter
    {
        ProtoCodes mCode;
        EoRAwaiter(ProtoCodes c) : mCode(c) {};
        bool await_ready() noexcept {
            if (mCode == ProtoCodes::EndOfRound)
            {
                mCode = success;
                return false;
            }
            return true;
        }
        void await_resume() noexcept {}
        void await_suspend(coro_handle const& handle) noexcept {}
    };
    auto await_transform(ProtoCodes res) noexcept {
        return EoRAwaiter((res));
    }

    //struct ECAwaiter
    //{
    //    ECAwaiter(std::error_code);
    //    void await_resume() noexcept;
    //    bool await_ready() noexcept;
    //    void await_suspend(coro_handle const& handle) noexcept;
    //};
    //auto await_transform(std::error_code res) noexcept {
    //    return ECAwaiter(res);
    //}

    RoundAwaiter await_transform(Round&& res) noexcept;

    //struct EcRoundAwaiter
    //{
    //    EcRoundAwaiter(EcRound&&);
    //    std::error_code await_resume() noexcept;
    //    bool await_ready() noexcept;
    //    void await_suspend(coro_handle const& handle) noexcept;
    //};
    //auto await_transform(EcRound&& res) noexcept {
    //    return EcRoundAwaiter(std::move(res));
    //}

    ProtocolAwaiter await_transform(Protocol& res) noexcept;
};




struct SendAwaiter
{
    using coro_handle = protocol_promise::coro_handle;
    //Send mSend;
    coro_handle mProt;
    std::error_code mEC;

    SendAwaiter(std::error_code e) : mEC(e) {}
    void await_resume() noexcept {}
    bool await_ready() noexcept { return true; }
    void await_suspend(coro_handle const& handle) noexcept {
        handle.promise().mEC = mEC;
    }
};

inline SendAwaiter protocol_promise::await_transform(Send&& res) noexcept
{
    try {
        getChannel().send(res.mBuff);
        return SendAwaiter({});
    }
    catch (...)
    {
        return SendAwaiter(ProtoCodes::BadRecv);
    }
}


struct RecvAwaiter
{
    using coro_handle = protocol_promise::coro_handle;
    coro_handle mProt;
    Recv mRecv;
    std::error_code mEC;
    std::future<void> mFu;
    RecvAwaiter(Recv&& r, coro_handle prot) : mRecv(std::move(r)), mProt(prot) {
        try {
            mFu = mProt.promise().getChannel().asyncRecv(mRecv.mBuff.data(), mRecv.mBuff.size());
        }
        catch (...)
        {
            mEC = ProtoCodes::BadRecv;
        }
    }

    void await_resume() noexcept { }
    bool await_ready() noexcept { 
        if (mEC)
            return false;
        auto status = mFu.wait_for(std::chrono::seconds(0));
        if (status == std::future_status::ready)
        {
            try {
                mFu.get();
            }
            catch (...)
            {
                mEC = ProtoCodes::BadRecv;
            }

            return !mEC;
        }
        else
        {
            mEC = ProtoCodes::PendingRecv;
            return false;
        }
    }
    void await_suspend(coro_handle const& handle) noexcept
    {
        mProt.promise().mEC = mEC;
        mEC = {};
    }
};
inline RecvAwaiter protocol_promise::await_transform(Recv&& res) noexcept {
    return RecvAwaiter(std::move(res), coro_handle::from_promise(*this));
}


struct Round
{
    using coro_handle = protocol_promise::coro_handle;
    coro_handle mProto;
    EcRound getErrorCode();

    Round setSendBuffer(ProtocolBuffer& b) { mProto.promise().mProtoBuff = &b; }
    //Round setRecvBuffer(ProtocolBuffer&);
};


struct RoundAwaiter
{
    using coro_handle = protocol_promise::coro_handle;
    coro_handle mProt, mParent;

    RoundAwaiter(coro_handle self, coro_handle parent) : mProt(self), mParent(parent) {}
    bool await_ready() noexcept
    {
        mProt.promise().mChl = mParent.promise().mChl;
        mProt.resume();
        return mProt.done() || mProt.promise().mEC == ProtoCodes::EndOfRound;
    }
    void await_resume() noexcept {}
    void await_suspend(coro_handle const& handle) noexcept
    {
        if (handle != mParent)
            panic("logic error " LOCATION);

        mParent.promise().propagateErrors(mProt.promise());
    }
};

inline RoundAwaiter protocol_promise::await_transform(Round&& res) noexcept {
    return RoundAwaiter(res.mProto, coro_handle::from_promise(*this));
}



struct ProtocolAwaiter
{
    using coro_handle = protocol_promise::coro_handle;
    coro_handle mProt, mParent;


    ProtocolAwaiter(coro_handle self, coro_handle parent) : mProt(self), mParent(parent) {}
    bool await_ready() noexcept {

        mProt.promise().mChl = mParent.promise().mChl;
        mProt.resume();

        return mProt.done();
    }

    std::error_code await_resume() noexcept
    {
        if (mProt.promise().mEx)
            std::rethrow_exception(mProt.promise().mEx);

        if (mProt.promise().mEC == ProtoCodes::EndOfRound)
            panic("logic error " LOCATION);

        return mProt.promise().mEC;
    }
    void await_suspend(coro_handle const& handle) noexcept
    {
        if (handle != mParent)
            panic("logic error " LOCATION);
    }
};









inline bool protocol_promise::hasError()
{
    return mEx ||
        (mEC && mEC != ProtoCodes::EndOfRound);
}
inline void protocol_promise::propagateErrors(protocol_promise& child)
{
    if (child.hasError())
    {
        mEC = child.mEC;
        mEx = std::move(child.mEx);
    }
}
inline auto protocol_promise::get_return_object() {
    return coro_handle::from_promise(*this);
}
inline auto protocol_promise::initial_suspend() { return std::experimental::suspend_always(); }
inline auto protocol_promise::final_suspend() noexcept { return std::experimental::suspend_always(); }
inline auto protocol_promise::return_value(std::error_code ec) {
    mEC = ec;
}
inline void protocol_promise::unhandled_exception() {
    mEx = std::current_exception();
}

















class Protocol
{
public:

    const static ProtoCodes endOfRound = ProtoCodes::EndOfRound;

    static Send send(std::vector<char>& v) { return Send(v); }
    static PBSend send(ProtocolBuffer& v) { return PBSend(v.mBuff); };
    //static Send send(std::vector<char>& v, PartyIdx);
    static Recv recv(std::vector<char>& v) { return Recv(v); }
    //static Recv recv(std::vector<char>& v, PartyIdx);

    static Initialize init(const std::string& n) { return { n }; };

    struct SID {};

    Round nextRound() { return { handle_ }; }



    using promise_type = protocol_promise;
    using coro_handle = promise_type::coro_handle;




    //bool await_ready() {
    //    return !handle_.done();
    //}
    //void await_suspend(stde::coroutine_handle<promise_type> coro_handle) {}

    //std::error_code await_resume() 
    //{

    //}

    Protocol(coro_handle handle) : handle_(handle) { assert(handle); }
    Protocol(Protocol&) = delete;
    Protocol(Protocol&&) = delete;

    std::error_code resume(Channel& chl) {
        handle_.promise().mChl = &chl;
        if (!handle_.done())
            handle_.resume();

        if (handle_.promise().mEx)
            std::rethrow_exception(handle_.promise().mEx);

        return handle_.promise().mEC;
        //return await_resume();
    }



    ~Protocol() { handle_.destroy(); }

    friend struct protocol_promise;
private:
    coro_handle handle_;
};

inline ProtocolAwaiter protocol_promise::await_transform(Protocol& res) noexcept {
    return ProtocolAwaiter(res.handle_, coro_handle::from_promise(*this));
}





inline Protocol foo(int partyIdx)
{
    co_await Protocol::init("foo()");

    if (partyIdx)
    {

        std::vector<char> buff(10);
        co_await Protocol::send(buff);
        
        co_await Protocol::endOfRound;

        co_await Protocol::recv(buff);
        co_await Protocol::send(buff);

        co_await Protocol::endOfRound;

        co_await Protocol::recv(buff);
        co_await Protocol::send(buff);
    }
    else
    {
        std::vector<char> buff(10);
        co_await Protocol::recv(buff);
        co_await Protocol::send(buff);

        co_await Protocol::endOfRound;

        co_await Protocol::recv(buff);
        co_await Protocol::send(buff);

        co_await Protocol::endOfRound;

        co_await Protocol::recv(buff);
    }

    co_return {};
}

inline Protocol bar(int pIdx)
{
    co_await Initialize("bar()");

    std::vector<char> data(10);
    co_await Protocol::send(data);
    co_await Protocol::recv(data);

    if (pIdx)
    {
        co_await Protocol::recv(data);

        Protocol subprotocol = foo(pIdx);

        // send
        co_await subprotocol.nextRound();

        co_await Protocol::endOfRound;

        co_await Protocol::recv(data);
        co_await Protocol::send(data);

        co_await Protocol::endOfRound;

        // recv,send, ..., send
        co_await subprotocol;

        co_await Protocol::endOfRound;

        co_await Protocol::recv(data);

    }
    else
    {
        Protocol::send(data);
        co_await Protocol::endOfRound;

        Protocol subprotocol = foo(pIdx);
        
        // recv
        ProtocolBuffer buff;
        co_await subprotocol.nextRound().setSendBuffer(buff);

        co_await Protocol::send(data);

        co_await Protocol::endOfRound;

        // recv, send
        co_await Protocol::recv(data);
        co_await Protocol::send(buff);

        // recv,send, ..., send
        co_await subprotocol;

        co_await Protocol::send(data);

    }

    co_return{};
}

inline void protoMain()
{
    auto proto0 = bar(0);
    auto proto1 = bar(1);


    oc::IOService ios;

    Channel chl0 = oc::Session(ios, "localhost:1212", oc::SessionMode::Client).addChannel();
    Channel chl1 = oc::Session(ios, "localhost:1212", oc::SessionMode::Server).addChannel();


    std::error_code code0 = Protocol::endOfRound;
    std::error_code code1 = Protocol::endOfRound;
    while (code0 || code1)
    {
        if (code0)
            code0 = proto0.resume(chl0);
        if (code1)
            code1 = proto0.resume(chl0);

        if (code0 == ProtoCatagory::Failure)
        {
            std::cout << "bad code0: " << code0 << std::endl;
        }
        if (code1 == ProtoCatagory::Failure)
        {
            std::cout << "bad code1: " << code1 << std::endl;
        }
    }
}
#pragma once
#define NNG_STATIC_LIB
#ifdef WIN32
#define NNG_COMMAND_SOCKET_PREFIX "ipc://ep-c-"
#define NNG_SAMPLES_SOCKET_PREFIX "ipc://ep-s-"
#else
#define NNG_COMMAND_SOCKET_PREFIX "ipc:///tmp/ep-c-"
#define NNG_SAMPLES_SOCKET_PREFIX "ipc:///tmp/ep-s-"
#endif

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/pair0/pair.h>
#include <assert.h>
#include <functional>
#include <string>
#include <vector>
#include <cstdlib>

class NngSelfRegister {
public:
    NngSelfRegister() : nngFinaliser{getNngFinaliser()} {}
    ~NngSelfRegister() {}

private:
    class NngFinaliser {
    public:
        NngFinaliser() {}
        ~NngFinaliser() { nng_fini(); }
    };

    std::shared_ptr<NngFinaliser> nngFinaliser;

    inline static std::weak_ptr<NngFinaliser> nngFinaliserStatic;

    static std::shared_ptr<NngFinaliser> getNngFinaliser() {
        if (std::shared_ptr<NngFinaliser> fini = nngFinaliserStatic.lock()) {
            return fini;
        } else {
            fini = std::make_shared<NngFinaliser>();
            nngFinaliserStatic = fini;
            return fini;
        }
    }
};

#define NNGMSG_DATACOUNT_STALE -1
#define NNGMSG_RESULT_UNKNOWN -1

class NngMsg {
public:
    NngMsg() {}

    NngMsg(size_t bufSz) { allocateBuffer(bufSz); }

    NngMsg(nng_msg* msg) {
        if (msg) {
            size_t bufSz = nng_msg_len(msg);
            allocateBuffer(bufSz);
            memcpy(buf, nng_msg_body(msg), bufSz);
            res = 0;
        }
    }

    ~NngMsg() {
        if (buf) freeBuffer();
    }

    void allocateBuffer(size_t bufSz) {
        buf = malloc(bufSz);
        sz = bufSz;
        dataCount = NNGMSG_DATACOUNT_STALE;  // Force recalc next request
    }

    void freeBuffer() { free(buf); }

    void* getBufferPointer() { return buf; }

    void* getBufferPointerPointer() { return &buf; }

    size_t* getSizePointer() {
        dataCount = NNGMSG_DATACOUNT_STALE;  // Force recalc next request
        return &sz;
    }

    size_t getSize() { return sz; }

    int getResult() { return res; }

    void setResult(int result) { res = result; }

    bool success() { return res == 0; }

protected:
    void* buf = NULL;
    size_t sz{0};
    int dataCount{NNGMSG_DATACOUNT_STALE};
    size_t dataSize{1};
    int res{NNGMSG_RESULT_UNKNOWN};

    void calcDataCount() {
        assert(sz % dataSize == 0);
        dataCount = (int)(sz / dataSize);
    }
};

template <typename T>
class TypedNngMsg : public NngMsg {
public:
    TypedNngMsg() : NngMsg() { dataSize = sizeof(T); }

    TypedNngMsg(int dataCount) : NngMsg() {
        dataSize = sizeof(T);
        size_t bufSz = dataCount * dataSize;
        allocateBuffer(bufSz);
    }

    ~TypedNngMsg() {}

    void advanceSeqReadPos(int dataPointsToAdvance = 1) {
        seqReadPos += dataPointsToAdvance;
    }

    bool seqReadAndPut(T* val, int numOfDataPoints = 1) {
        if (getDataAt(val, seqReadPos, numOfDataPoints)) {
            seqReadPos += numOfDataPoints;
            return true;
        }
        return false;
    }

    size_t getDataCount() {
        if (dataCount == NNGMSG_DATACOUNT_STALE) calcDataCount();
        return dataCount;
    }

    int getSeqReadPos() { return seqReadPos / dataSize; }

    bool atSeqReadEnd() { return (!buf) || (seqReadPos >= getDataCount()); }

    T* getSeqReadPointer() {
        if (atSeqReadEnd()) return nullptr;
        T* ret = (T*)buf + seqReadPos;
        return ret;
    }

    bool getDataAt(T* val, int startingDataPoint, int numOfDataPoints = 1) {
        assert((startingDataPoint + numOfDataPoints) <= getDataCount());
        if (startingDataPoint < 0 ||
            (startingDataPoint + numOfDataPoints) > getDataCount() || !buf)
            return false;
        memcpy(val, (T*)buf + startingDataPoint, dataSize * numOfDataPoints);
        return true;
    }

private:
    int seqReadPos{0};
};

#define NNG_PORT_UNKNOWN 0
#define NNG_PORT_START 1
#define NNG_PORT_END 65535

class SocketBase {
public:
    SocketBase() : nngSelfRegister{std::make_shared<NngSelfRegister>()} {}
    ~SocketBase() {}

    int getPort() {
        assert(port != NNG_PORT_UNKNOWN);
        return port;
    }

    bool isSocketOpen() { return socketOpen; }

    int open() {
        if (socketOpen) {
            return 0;
        } else {
            auto ret = openSpecifics();
            socketOpen = (ret == 0);
            return ret;
        }
    }

    int close() {
        if (socketOpen) {
            auto ret = closeSpecifics();
            socketOpen = !(ret == 0);
            if (!socketOpen) port = NNG_PORT_UNKNOWN;
            return ret;
        } else {
            return 0;
        }
    }

    friend class SocketBaseForListeners;
    friend class SocketBaseForDiallers;

protected:
    nng_socket socket;

    virtual int openSpecifics() = 0;
    virtual int closeSpecifics() {
        // This'll suit most cases
        auto resClose = nng_close(socket);
        assert(resClose == 0);
        return resClose;
    }

    void setPort(int newPortNum) {
        if (newPortNum >= NNG_PORT_START && newPortNum <= NNG_PORT_END) {
            port = newPortNum;
        } else {
            port = NNG_PORT_UNKNOWN;
        }
    }

private:
    std::shared_ptr<NngSelfRegister> nngSelfRegister;
    int port{0};
    bool socketOpen{false};
};

class SocketBaseForListeners : public SocketBase {
public:
    SocketBaseForListeners() : SocketBase(){};
    ~SocketBaseForListeners(){};

    int listen() {
        auto ret = listenSpecifics();
        if (ret != 0) port = NNG_PORT_UNKNOWN;
        return ret;
    }

protected:
    virtual int listenSpecifics() = 0;
};

class SocketBaseForDiallers : public SocketBase {
public:
    SocketBaseForDiallers() : SocketBase(){};
    ~SocketBaseForDiallers(){};

    int dial(int portNum) {
        auto ret = dialSpecifics(portNum);
        port = (ret == 0) ? portNum : NNG_PORT_UNKNOWN;
        return ret;
    }

protected:
    virtual int dialSpecifics(int portNum) = 0;
};

class CommandCommon {
public:
    enum Command { GetConfig, StartRender, StopRender, GetAdmAndMappings, SetAdmAndMappings };
};

class CommandReceiver : public SocketBaseForListeners, public CommandCommon {
public:
    using callback = std::function<void(std::shared_ptr<NngMsg>)>;

    CommandReceiver(callback commandHandler)
        : SocketBaseForListeners(), commandHandler{commandHandler} {}
    ~CommandReceiver() {}

    void sendResp(uint8_t respCode) {
        nng_msg* msg;
        nng_msg_alloc(&msg, 1);
        memcpy(nng_msg_body(msg), &respCode, 1);
        nng_aio_set_msg(aio, msg);
    }

    void sendAdmAndMappings(std::string admStr, std::vector<uint32_t> channelToAtuMapping) {
        assert(channelToAtuMapping.size() == 64);
        nng_msg* msg;

        uint32_t admSz = admStr.size();
        uint32_t mapSz = (64 * 4);
        uint32_t totSz = mapSz + admSz;
        nng_msg_alloc(&msg, totSz);

        char* bufPtr = (char*)nng_msg_body(msg);
        int bufOffset = 0;

        memcpy(bufPtr + bufOffset, channelToAtuMapping.data(), mapSz);
        bufOffset += mapSz;
        memcpy(bufPtr + bufOffset, admStr.data(), admSz);
        bufOffset += admSz;

        assert(totSz == bufOffset);
        nng_aio_set_msg(aio, msg);
    }

    void sendInfo(uint8_t channels, uint32_t sampleRate,
                  uint16_t admTypeDefinition = 0, uint16_t admPackFormatId = 0,
                  uint16_t admChannelFormatId = 0) {
        nng_msg* msg;
        nng_msg_alloc(&msg, 11);
        char* bufPtr = (char*)nng_msg_body(msg);
        memcpy(bufPtr + 0, &channels, 1);
        memcpy(bufPtr + 1, &sampleRate, 4);
        memcpy(bufPtr + 5, &admTypeDefinition, 2);
        memcpy(bufPtr + 7, &admPackFormatId, 2);
        memcpy(bufPtr + 9, &admChannelFormatId, 2);
        nng_aio_set_msg(aio, msg);
    }

    void handleAioCallback() {
        int resAioRes;
        nng_msg* nngMsg;

        switch (currentAioState) {
            case INIT:
                currentAioState = RECV;
                nng_ctx_recv(ctx, aio);
                break;

            case RECV:
                resAioRes = nng_aio_result(aio);
                if (resAioRes == NNG_ECANCELED) break;
                assert(resAioRes == 0);

                nngMsg = nng_aio_get_msg(aio);
                if (nngMsg) {
                    auto msg = std::make_shared<NngMsg>(nngMsg);
                    commandHandler(std::move(msg));
                    nng_msg_free(nngMsg);
                }

                currentAioState = WAIT;
                nng_sleep_aio(1, aio);
                break;

            case WAIT:
                currentAioState = SEND;
                nng_ctx_send(ctx, aio);
                break;

            case SEND:
                resAioRes = nng_aio_result(aio);
                assert(resAioRes == 0);

                currentAioState = RECV;
                nng_ctx_recv(ctx, aio);
                break;
        }
    }

    static void internalAioCallback(void* arg) {
        auto cst = static_cast<CommandReceiver*>(arg);
        cst->handleAioCallback();
    }

private:
    enum aioState { INIT, RECV, WAIT, SEND };
    nng_aio* aio{nullptr};
    aioState currentAioState{aioState::INIT};
    callback commandHandler{nullptr};
    nng_ctx ctx;

    int openSpecifics() override {
        auto resOpen = nng_rep_open(&socket);
        assert(resOpen == 0);

        if (resOpen == 0) {
            auto resSize =
                nng_setopt_size(socket, NNG_OPT_RECVMAXSZ,
                                0);  // Enforce no limit on message size (for now)
            assert(resSize == 0);
            auto resBuf = nng_setopt_int(socket, NNG_OPT_RECVBUF,
                                         1000);  // Max messages sitting in buffer
            assert(resBuf == 0);
            auto resAioAlloc =
                nng_aio_alloc(&aio, CommandReceiver::internalAioCallback, this);
            assert(resAioAlloc == 0);
            auto resCtxOpen = nng_ctx_open(&ctx, socket);
            assert(resCtxOpen == 0);
            currentAioState = INIT;
            handleAioCallback();  // Starts loop
        }

        return resOpen;
    }

    int closeSpecifics() override {
        if (aio) {
            nng_aio_stop(aio);
            nng_aio_free(aio);
            aio = nullptr;
        }
        auto resCtxClose = nng_ctx_close(ctx);
        assert(resCtxClose == 0);
        auto resClose = nng_close(socket);
        assert(resClose == 0);
        return resClose;
    }

    int listenSpecifics() override {
        std::string baseUrl = NNG_COMMAND_SOCKET_PREFIX;
        int portSuffix = NNG_PORT_START;
        std::string fullUrl = baseUrl + std::to_string(portSuffix);
        int res;
        while (portSuffix < NNG_PORT_END &&
            (res = nng_listen(socket, fullUrl.c_str(), NULL, 0)) ==
               NNG_EADDRINUSE) {
            portSuffix++;
            fullUrl = baseUrl + std::to_string(portSuffix);
        }
        if (portSuffix < NNG_PORT_END && res == 0) {
            setPort(portSuffix);
        } else {
            setPort(NNG_PORT_UNKNOWN);
        }
        return res;
    }
};

class CommandSender : public SocketBaseForDiallers, public CommandCommon {
public:
    CommandSender() : SocketBaseForDiallers() {}
    ~CommandSender() {}

    std::shared_ptr<NngMsg> doCommand(Command cmd) {
        nng_msg* msg;
        uint8_t cmdInt = (uint8_t)cmd;
        auto resMsg = nng_msg_alloc(&msg, 0);
        assert(resMsg == 0);
        resMsg = nng_msg_append(msg, &cmdInt, 1);
        assert(resMsg == 0);
        resMsg = nng_sendmsg(socket, msg, 0);
        assert(resMsg == 0);
        resMsg = nng_recvmsg(socket, &msg, 0);
        assert(resMsg == 0);
        auto nngMsg = std::make_shared<NngMsg>(msg);
        nng_msg_free(msg);
        return std::move(nngMsg);
    }

    std::shared_ptr<NngMsg> sendAdmAndMappings(std::string admStr, std::vector<uint32_t> channelToAtuMapping) {
        assert(channelToAtuMapping.size() == 64);
        nng_msg* msg;

        uint8_t cmdInt = (uint8_t)Command::SetAdmAndMappings;
        uint32_t admSz = admStr.size();
        uint32_t mapSz = (64 * 4);
        uint32_t totSz = 1 + mapSz + admSz;
        nng_msg_alloc(&msg, totSz);

        char* bufPtr = (char*)nng_msg_body(msg);
        int bufOffset = 0;
        memcpy(bufPtr + bufOffset, &cmdInt, 1);
        bufOffset += 1;
        memcpy(bufPtr + bufOffset, channelToAtuMapping.data(), mapSz);
        bufOffset += mapSz;
        memcpy(bufPtr + bufOffset, admStr.data(), admSz);
        bufOffset += admSz;

        assert(totSz == bufOffset);

        auto resMsg = nng_sendmsg(socket, msg, 0);
        assert(resMsg == 0);
        resMsg = nng_recvmsg(socket, &msg, 0);
        assert(resMsg == 0);
        auto nngMsg = std::make_shared<NngMsg>(msg);
        nng_msg_free(msg);
        return std::move(nngMsg);
    }

private:
    int openSpecifics() override {
        auto resOpen = nng_req_open(&socket);
        assert(resOpen == 0);
        return resOpen;
    }

    int dialSpecifics(int dialPortNum) override {
        std::string url = NNG_COMMAND_SOCKET_PREFIX;
        url.append(std::to_string(dialPortNum));
        auto res = nng_dial(socket, url.c_str(), NULL, 0);
        assert(res == 0);
        if (res == 0) setPort(dialPortNum);
        return res;
    }
};

class SamplesSender : public SocketBaseForListeners {
public:
    SamplesSender() : SocketBaseForListeners() {}
    ~SamplesSender() {}

    int sendBlock(std::shared_ptr<NngMsg> msg, int timeOut = 0) {
        nng_setopt_ms(socket, NNG_OPT_SENDTIMEO, timeOut);
        return nng_send(socket, msg->getBufferPointer(), msg->getSize(),
            (timeOut == 0) ? NNG_FLAG_NONBLOCK : 0);
    }

private:
    int openSpecifics() override {
        auto resOpen = nng_pair_open(&socket);
        assert(resOpen == 0);

        if (resOpen == 0) {
            auto resSize =
                nng_setopt_size(socket, NNG_OPT_RECVMAXSZ,
                                0);  // Enforce no limit on message size (for now)
            assert(resSize == 0);
            auto resBuf = nng_setopt_int(socket, NNG_OPT_RECVBUF,
                                         1000);  // Max messages sitting in buffer
            assert(resBuf == 0);
            resBuf = nng_setopt_int(socket, NNG_OPT_SENDBUF,
                                    1000);  // Max messages sitting in buffer
            assert(resBuf == 0);
        }

        return resOpen;
    }

    int listenSpecifics() override {
        std::string baseUrl = NNG_SAMPLES_SOCKET_PREFIX;
        int portSuffix = NNG_PORT_START;
        std::string fullUrl = baseUrl + std::to_string(portSuffix);
        int res;
        while (portSuffix < NNG_PORT_END &&
            (res = nng_listen(socket, fullUrl.c_str(), NULL, 0)) ==
               NNG_EADDRINUSE) {
            portSuffix++;
            fullUrl = baseUrl + std::to_string(portSuffix);
        }
        if (portSuffix < NNG_PORT_END && res == 0) {
            setPort(portSuffix);
        } else {
            setPort(NNG_PORT_UNKNOWN);
        }
        return res;
    }
};

class SamplesReceiver : public SocketBaseForDiallers {
public:
    SamplesReceiver() : SocketBaseForDiallers() {}
    ~SamplesReceiver() {}

    std::shared_ptr<TypedNngMsg<float>> receiveBlock() {
        auto msg = std::make_shared<TypedNngMsg<float>>();
        msg->setResult(
            nng_recv(socket, msg->getBufferPointerPointer(), msg->getSizePointer(),
                     NNG_FLAG_ALLOC));  // NNG_FLAG_NONBLOCK | NNG_FLAG_ALLOC));
        return std::move(msg);
    }

private:
    int openSpecifics() override {
        auto resOpen = nng_pair_open(&socket);
        assert(resOpen == 0);

        if (resOpen == 0) {
            auto resSize =
                nng_setopt_size(socket, NNG_OPT_RECVMAXSZ,
                                0);  // Enforce no limit on message size (for now)
            assert(resSize == 0);
            auto resBuf = nng_setopt_int(socket, NNG_OPT_RECVBUF,
                                         1000);  // Max messages sitting in buffer
            assert(resBuf == 0);
            resBuf = nng_setopt_int(socket, NNG_OPT_SENDBUF,
                                    1000);  // Max messages sitting in buffer
            assert(resBuf == 0);
            auto resTimeO = nng_setopt_ms(socket, NNG_OPT_RECVTIMEO, 100);
            assert(resTimeO == 0);
        }

        return resOpen;
    }

    int dialSpecifics(int dialPortNum) override {
        std::string url = NNG_SAMPLES_SOCKET_PREFIX;
        url.append(std::to_string(dialPortNum));
        auto res = nng_dial(socket, url.c_str(), NULL, 0);
        assert(res == 0);
        if (res == 0) setPort(dialPortNum);
        return res;
    }
};

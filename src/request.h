//
// Created by 梁俊华 on 11/25/22.
//

#ifndef REPCREC_REQUEST_H
#define REPCREC_REQUEST_H

#include "global.h"

#include <memory>

class Request {
public:
    explicit Request(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id = -1, repcrec::site_id_t site_id = -1, repcrec::var_id_t var_id = -1);
    virtual ~Request() = default;

    virtual void exec() = 0;

protected:
    repcrec::timestamp_t timestamp_;
    repcrec::tran_id_t tran_id_;
    repcrec::site_id_t site_id_;
    repcrec::var_id_t var_id_;
};

class WriteRequest : public Request {
public:
    WriteRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id = -1, repcrec::var_id_t var_id = -1, repcrec::var_t value = INT_MIN);
    ~WriteRequest() override = default;

    void exec() override;

private:
    repcrec::var_t value_;
};

class ReadRequest : public Request {
public:
    ReadRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::var_id_t var_id = -1);
    ~ReadRequest() override = default;

    void exec() override;
};

class CreateTransactionRequest : public Request {
public:
    CreateTransactionRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, bool is_read_only, repcrec::site_id_t site_id = -1, repcrec::var_id_t var_id = -1);
    ~CreateTransactionRequest() override = default;

    void exec() override;

private:
    bool is_read_only_;
};

class EndTransactionRequest : public Request {
public:
    EndTransactionRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id = -1, repcrec::var_id_t var_id = -1);
    ~EndTransactionRequest() override = default;

    void exec() override;
};

class DumpRequest : public Request {
public:
    explicit DumpRequest(repcrec::timestamp_t timestamp);
    ~DumpRequest() override = default;

    void exec() override;
};

class FailRequest : public Request {
public:
    FailRequest(repcrec::timestamp_t timestamp, repcrec::site_id_t site_id);
    ~FailRequest() override = default;

    void exec() override;
};

class RecoveryRequest : public Request {
public:
    RecoveryRequest(repcrec::timestamp_t timestamp, repcrec::site_id_t site_id);
    ~RecoveryRequest() override = default;

    void exec() override;
};

#endif//REPCREC_REQUEST_H

// The MIT License (MIT)
//
// Copyright (c) 2022 Shane Butler
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "fsm.h"
#include "traits.h"

#include <cstdint>
#include <variant>
#include <optional>
#include <stdexcept>
#include <memory>
#include <span>
#include <thread>

#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>


namespace cl {

constexpr std::size_t DEFAULT_MAX_READ_BUFFER_SIZE = 65535;


using io_service_t   = boost::asio::io_service;
using address_t      = boost::asio::ip::address;
using strand_t       = boost::asio::io_service::strand;
using strand_ptr_t   = std::shared_ptr<strand_t>;

// todo: need a wrqp for std::error_code and boosts error_code impl -> 
//          for now we just alias and only handle the boost error_codes (no 
//                codes for custom errors, e.g., cl::client state transition error )

using error_code_t   = boost::system::error_code;
using udp_socket_t   = boost::asio::ip::udp::socket;
using udp_endpoint_t = boost::asio::ip::udp::endpoint;
using udp_resolver_t = boost::asio::ip::udp::resolver;
using tcp_socket_t   = boost::asio::ip::tcp::socket;
using tcp_endpoint_t = boost::asio::ip::tcp::endpoint;
using tcp_resolver_t = boost::asio::ip::tcp::resolver;

using port_t = std::uint16_t;

using data_t               = std::vector<std::uint8_t>;
using on_recieve_handler_t = std::function<void(data_t)>;
using on_error_handler_t   = std::function<void(const error_code_t& ec)>;


struct endpoint {

    std::uint16_t port;
    std::optional<std::string> address;

    template<typename Endpoint>
    requires cl::traits::is_supported_endpoint_type<Endpoint>
    [[nodiscard]] operator Endpoint() const {
        return address ? Endpoint(address_t::from_string(*address), port) : Endpoint(Endpoint::protocol_type::v4(), port);
    }
};


struct event_set_endpoints {

    // todo: resolution when only server(sender) is given -> at the minute both ports must be set.
    // event_set_endpoints(std::uint16_t p) 
    //     : sender(std::make_unique<endpoint>(p))
    // { }

    // event_set_endpoints(std::uint16_t p, std::string a) 
    //     : sender(std::make_unique<endpoint>(p, a))
    // { }

    // event_set_endpoints(endpoint se) 
    //     : sender(std::make_unique<endpoint>(std::move(se)))
    // { }

    event_set_endpoints(endpoint se, endpoint ce) 
        : sender(std::make_unique<endpoint>(std::move(se))) 
        , reciever(std::make_unique<endpoint>(std::move(ce)))
    { }

    std::unique_ptr<endpoint> sender;
    std::unique_ptr<endpoint> reciever;
};


struct event_set_on_recieve_handler {
    on_recieve_handler_t onrecv;
    template<typename... Args>
    event_set_on_recieve_handler(Args&& ... args) 
        : onrecv(std::forward<Args>(args)...)
    { }    
};

struct event_start {};

/* todo: should be private */
struct event_io_service_stopped {};

struct event_write_request {
    std::shared_ptr<data_t> payload;
    template<typename... Args>
    event_write_request(Args&& ... args) 
        : payload(std::make_shared<data_t>(std::forward<Args>(args)...))
    { }  
};

struct event_stop {};

struct state_awaiting_subscription {};
struct state_idle {};
struct state_ready {};
struct state_running {};
struct state_stopping {};
struct state_stopped {};
struct state_shutdown {};

using state_t = std::variant<
    state_idle, 
    state_awaiting_subscription,
    state_ready,
    state_running,
    state_stopped
>;

using optional_state_t = std::optional<state_t>;


template<class Socket>
class client : public fsm::fsm<client<Socket>, state_t> {
public:
    client() = default;
    client(const client&) = delete;
    client& operator=(const client&) = delete;
    client(client&&) = delete;
    client& operator=(client&&) = delete;

    
    ~client(){ }
    /* deduce endpoint type for storing dest address, and shorthands for others */
    using client_t              = client<Socket>;
    using client_endpoint_t     = cl::traits::endpoint_type_from_socket_t<Socket>;
    using client_socket_t       = cl::traits::socket_type_identity_t<Socket>;
    using client_protocol_t     = cl::traits::protocol_type_from_socket_t<Socket>;
    using client_resolver_t     = cl::traits::resolver_type_from_socket_t<Socket>;

    using client_endpoint_ptr_t = std::shared_ptr<client_endpoint_t>;
    using client_socket_ptr_t   = std::shared_ptr<client_socket_t>;

    template<typename State, typename Event>
    auto on_event(State&, const Event&) -> optional_state_t {
        // todo: need an error_code to wrap boost/std error_code -> for now throw for invalid transition
        throw std::logic_error("Invalid transition");
    }

    auto on_event(state_idle&, const event_set_endpoints& e) -> optional_state_t {
        // todo: at the minute both the sender and reciever must set port -> look at endpoint event and resolution to allow other options
        socket_ = std::make_shared<client_socket_t>(io_service_, *e.reciever);
        endpoint_ = *e.sender;
        return state_awaiting_subscription();
    }

    auto on_event(state_awaiting_subscription&, const event_set_on_recieve_handler& e) -> optional_state_t { 
        onrecv_ = std::make_unique<on_recieve_handler_t>(std::move(e.onrecv));
        return state_ready();
    }

    auto on_event(state_ready&, const event_start&) -> optional_state_t { 
        // todo: template away this if onrecv
        con_ = connection::create(socket_, strand_t(io_service_), strand_t(io_service_), endpoint_, 
            [this] (const error_code_t& ec) { if(onerror_)(*onerror_)(ec);  }, 
            [this] (data_t&& payload) mutable { if(onrecv_)(*onrecv_)(std::move(payload)); }
        );
        con_->start();
        start_io_thread();
        return state_running();
    }

    auto on_event(state_running&, const event_write_request& e) -> optional_state_t { 
        con_->write(std::move(e.payload));
        return std::nullopt;
    }
    auto on_event(state_running&, const event_stop& e) -> optional_state_t { 
        io_thread_.request_stop();
        if(con_) con_->stop();
        // todo: shutdown on already shutdown service?
        io_service_.shutdown();
        if(con_) con_.reset();
        return std::nullopt;
    }

    auto on_event(auto&, const event_io_service_stopped&) -> optional_state_t { io_thread_.request_stop(); std::nullopt; }

    template<typename State>
    requires traits::is_any_of<state_running, state_ready>
    auto on_event(state_running&, const event_io_service_stopped&) -> optional_state_t { 
        return std::nullopt;
    } 

private:

  class connection : public std::enable_shared_from_this<connection> {

    public:

        using self_ptr_t        = std::shared_ptr<connection>;
        using data_ptr_t        = std::shared_ptr<data_t>;
        using onwrite_handler_t = std::function<void(std::size_t, const error_code_t& ec)>;
        using onrecv_handler_t  = std::function<std::size_t(std::span<const std::uint8_t>, const error_code_t& ec)>;

        connection(const connection&) = delete;
        connection& operator=(const connection&) = delete;
        connection(connection&&) = delete;
        connection& operator=(connection&&) = delete;

        // todo: look at the coping/moving/ptr-setup in ctor/members

        [[nodiscard]] static auto create(
            client_socket_ptr_t socket, 
            strand_t socket_strand, 
            strand_t recv_strand, 
            client_endpoint_t sender,
            on_error_handler_t&& onerror,
            on_recieve_handler_t&& onrecv) -> self_ptr_t {
            return std::shared_ptr<connection>(new connection(std::move(socket), 
                                                             std::move(socket_strand), 
                                                             std::move(recv_strand), 
                                                             sender, 
                                                             std::move(onerror), 
                                                             std::move(onrecv)));
        }

        inline auto start() -> void { do_receive(); }
        inline auto stop() -> void {
            if(socket_.is_open()) {
                socket_.shutdown(client_socket_t::shutdown_both);
                socket_.close();
            }
            socket_.reset();
        }
        inline auto write(data_ptr_t p) -> void { do_write(std::move(p)); }

    private:

        inline connection(
            client_socket_ptr_t socket, 
            strand_t socket_strand, 
            strand_t recv_strand, 
            client_endpoint_t sender, 
            on_error_handler_t&& onerror,
            on_recieve_handler_t&& onrecv,
            std::size_t max_buffer_size
        ) noexcept
                : socket_(std::move(socket))
                , socket_strand_(std::move(socket_strand))
                , recv_handle_strand_(std::move(recv_strand))
                , endpoint_(sender)
                , onerror_(std::make_unique<on_error_handler_t>(std::move(onerror)))
                , onrecv_(std::make_unique<on_recieve_handler_t>(std::move(onrecv)))
                , inbuf_(max_buffer_size)
                , outbuf_(max_buffer_size)
        { }

        // inline explicit connection(io_service_t& io, client_socket_t socket, client_endpoint_t sender) noexcept
        inline connection(
            client_socket_ptr_t socket, 
            strand_t socket_strand, 
            strand_t recv_strand, 
            client_endpoint_t sender, 
            on_error_handler_t&& onerror,
            on_recieve_handler_t&& onrecv
        ) noexcept
                : socket_(std::move(socket))
                , socket_strand_(std::move(socket_strand))
                , recv_handle_strand_(std::move(recv_strand))
                , endpoint_(sender)
                , onerror_(std::make_unique<on_error_handler_t>(std::move(onerror)))
                , onrecv_(std::make_unique<on_recieve_handler_t>(std::move(onrecv)))
                , inbuf_(DEFAULT_MAX_READ_BUFFER_SIZE)
                , outbuf_(DEFAULT_MAX_READ_BUFFER_SIZE)
        { }

        auto do_receive() -> void {
            if(!socket_) return;
            socket_->async_receive_from(boost::asio::buffer(inbuf_.data(), inbuf_.size()),
                                       endpoint_,
                                       socket_strand_.wrap(boost::bind(&connection::on_recieve, 
                                                                 this, 
                                                                 this->shared_from_this(),
                                                                 boost::asio::placeholders::error, 
                                                                 boost::asio::placeholders::bytes_transferred)));

        }

        // todo: assuming fully fledged udp packets (no partials) this buffering can be templated out for udp; -> 
        //          if always full/complete udp messages what is the general max size?

        auto on_recieve(self_ptr_t, const error_code_t& ec, std::size_t sz) {
            if(ec) {
                dispatch_error(ec);
                return;
            }
            dispatch_recv(std::span(inbuf_.data(), sz));
            do_receive();
        }

        void dispatch_recv(std::span<std::uint8_t> sp) {
            recv_handle_strand_.post([self=this->shared_from_this(), payload=data_t{sp.begin(), sp.end()}]() mutable {
                (*self->onrecv_)(std::move(payload));
            });        
        }

        void dispatch_error(const error_code_t& ec) {
            recv_handle_strand_.post([self=this->shared_from_this(), ec](){
                (*self->onerror_)(ec);
            });        
        }

        auto do_write(data_ptr_t dp) {
            if(!socket_) return;
            socket_->async_send_to(boost::asio::buffer(dp->data(), dp->size()), 
                                  endpoint_,
                                  socket_strand_.wrap(boost::bind(&connection::on_write, this, 
                                                            this->shared_from_this(), dp,  // keep-alive of buffer/endpoint
                                                            boost::asio::placeholders::error, 
                                                            boost::asio::placeholders::bytes_transferred)));
        }

        auto on_write(self_ptr_t, data_ptr_t, const error_code_t& ec, std::size_t /*bytes_transferred*/) {
            if(ec) dispatch_error(ec);
        }

        client_socket_ptr_t socket_;
        strand_t socket_strand_;
        strand_t recv_handle_strand_;
        client_endpoint_t endpoint_;
        std::unique_ptr<on_error_handler_t> onerror_;
        std::unique_ptr<on_recieve_handler_t> onrecv_;

        std::vector<std::uint8_t> inbuf_;
        std::vector<std::uint8_t> outbuf_;

        std::size_t buffered_ = 0;

  };

  auto start_io_thread() -> void {
      if(io_thread_.joinable()) return;
      io_thread_ = std::jthread(
            [this](std::stop_token s) { 
                while(!s.stop_requested()) { 
                    io_service_.run(); 
                    this->dispatch(event_io_service_stopped{});
                }; 
            } 
      );
      io_thread_.detach();
  }

  io_service_t io_service_;
  client_socket_ptr_t socket_;
  client_endpoint_t endpoint_;
  std::unique_ptr<on_recieve_handler_t> onrecv_;
  std::unique_ptr<on_error_handler_t> onerror_;
  //template<typename T>
  std::shared_ptr<client_t::connection> con_;
  std::jthread io_thread_;
};
} // sbc::client

/*
            // if(!buffered_ && (buffered_ = (sz - onrecv_(std::span(inbuf_.data(), sz), ec)))) 
            //     std::memcpy(outbuf_.data(), inbuf_.data() + (sz - buffered_), buffered_);
            // else 
            //     buffering(ec, sz);
        auto buffering(const error_code_t& ec, std::size_t sz ) -> void {
            if(ec) {
                dispatch_error(ec);
                return;
            }

            // const std::size_t total = sz + buffered_;
            // buffer_append(outbuf_, inbuf_, sz);

            // if ((buffered_ = (total - onrecv_(std::span(outbuf_.data(), total), ec)))) {
            //     std::memcpy(outbuf_.data(), inbuf_.data() + sz, buffered_);
            // }
        }

        auto buffer_ensure(data_t& buf, const std::size_t required) -> void {
            if(buf.size() < required) buf.resize(buf.size() + (buf.size() - required));
        }

        auto buffer_append(data_t& dst, const data_t& src, const std::size_t sz ) {
            buffer_ensure(dst, sz + buffered_);
            std::memcpy(dst.data() + buffered_, src.data(), sz);
        }
*/


  
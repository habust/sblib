// // The MIT License (MIT)
// //
// // Copyright (c) 2022 Shane Butler
// //
// // Permission is hereby granted, free of charge, to any person obtaining a copy
// // of this software and associated documentation files (the "Software"), to deal
// // in the Software without restriction, including without limitation the rights
// // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// // copies of the Software, and to permit persons to whom the Software is
// // furnished to do so, subject to the following conditions:
// //
// // The above copyright notice and this permission notice shall be included in all
// // copies or substantial portions of the Software.
// //
// // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// // SOFTWARE.

// #pragma once

// #include "traits.h"

// #include <vector>
// #include <optional>
// #include <variant>

// #include <boost/asio.hpp>

// namespace cl::def {

// constexpr std::size_t DEFAULT_MAX_READ_BUFFER_SIZE = 65535;


// using io_service_t   = boost::asio::io_service;
// using address_t      = boost::asio::ip::address;
// using strand_t       = boost::asio::io_service::strand;

// // todo: need a wrqp for std::error_code and boosts error_code impl -> 
// //          for now we just alias and only handle the boost error_codes (no 
// //                codes for custom errors, e.g., cl::client state transition error )

// using error_code_t   = boost::system::error_code;
// using udp_socket_t   = boost::asio::ip::udp::socket;
// using udp_endpoint_t = boost::asio::ip::udp::endpoint;
// using udp_resolver_t = boost::asio::ip::udp::resolver;
// using tcp_socket_t   = boost::asio::ip::tcp::socket;
// using tcp_endpoint_t = boost::asio::ip::tcp::endpoint;
// using tcp_resolver_t = boost::asio::ip::tcp::resolver;


// using data_t               = std::vector<std::uint8_t>;
// using on_recieve_handler_t = std::function<std::uint8_t(data_t)>;
// using on_error_handler_t   = std::function<void(const error_code_t& ec)>;


// struct endpoint {

//     std::uint16_t port;
//     std::optional<std::string> address;

//     template<typename Endpoint>
//     requires cl::traits::is_supported_endpoint_type<Endpoint>
//     [[nodiscard]] operator Endpoint() const {
//         return address ? Endpoint(address_t::from_string(*address), port) : Endpoint(Endpoint::protocol_type::v4(), port);
//     }
// };


// //struct state_info;
// struct event_set_endpoints;
// struct event_add_on_recieve_handler;
// //struct event_add_on_recieve_handler;

// // struct event_connected;
// // struct event_disconnect;
// // struct event_timeout;
// // struct event_timeout;


// struct state_idle;
// ///struct state_setting_endpoints;
// //struct state_endpoints_set;
// struct state_awaiting_subscription;
// //struct state_on_revieve_subscription_added;
// //struct state_awaiting_endpoints;
// struct state_ready;

// // struct state_connecting;
// // struct state_connected_no_subscriptions;
// // struct state_connected_with_subscriptions;
// // struct state_adding_on_recv_subscription;
// // struct state_removing_on_recv_subscription;
// // struct state_setting_on_error_subscription;
// // struct state_subscription_added;
// // struct state_subscription_removed;
// // struct state_disconnecting;
// // struct state_disconnected;


// using state_t = std::variant<
//     state_idle, 
//     //state_setting_endpoints,
//     state_awaiting_subscription,
//     //state_endpoints_set,
//     //state_on_revieve_subscription_added,
//     //state_awaiting_endpoints,
//     state_ready
//     // state_connecting, 
//     // state_connected_no_subscriptions, 
//     // state_connected_with_subscriptions, 
//     // state_adding_on_recv_subscription, 
//     // state_removing_on_recv_subscription, 
//     // state_setting_on_error_subscription, 
//     // state_subscription_added, 
//     // state_subscription_removed, 
//     // state_disconnected, 
//     // state_disconnecting
// >;

// using optional_state_t = std::optional<state_t>;

// } // sbc
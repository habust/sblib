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

#include <concepts>

#include <boost/asio.hpp>

namespace cl::traits {

  template<typename T, typename... U>
  concept is_any_of = (std::same_as<T, U> || ...);

  template<typename T>
  concept is_supported_endpoint_type = is_any_of<T, boost::asio::ip::udp::endpoint, boost::asio::ip::tcp::endpoint>;

  template<typename T>
  concept is_endpoint_convertable = is_any_of<T, boost::asio::ip::udp::endpoint, boost::asio::ip::tcp::endpoint>;

  template<typename T>
  concept is_supported_socket_type = is_any_of<T, boost::asio::ip::udp::socket, boost::asio::ip::tcp::socket>;

  template<typename S, typename E>
  concept is_udp_socket_endpoint_compatible = std::same_as<S, boost::asio::ip::udp::socket>&& std::same_as<E, boost::asio::ip::udp::endpoint>;

  template<typename E>
  concept is_tcp_endpoint = std::same_as<E, boost::asio::ip::tcp::endpoint>;
  
  template<typename S>
  concept is_tcp_socket = std::same_as<S, boost::asio::ip::tcp::socket>;

  template<typename S, typename E>
  concept is_tcp_socket_endpoint_compatible = is_tcp_socket<S> && is_tcp_endpoint<E>;

  template<typename S, typename E>
  concept is_socket_endpoint_compatible = is_udp_socket_endpoint_compatible<S, E> || is_tcp_socket_endpoint_compatible<S, E>;

  template<typename E>
  requires is_supported_endpoint_type<E> 
  using endpoint_type_identity_t = typename std::type_identity_t<E>;

  template<typename E>
  requires is_supported_socket_type<E> 
  using socket_type_identity_t = typename std::type_identity_t<E>;

  template<typename S>
  struct endpoint_type_from_socket;

  template<>
  struct endpoint_type_from_socket<boost::asio::ip::tcp::socket> { using type = boost::asio::ip::tcp::endpoint; };

  template<>
  struct endpoint_type_from_socket<boost::asio::ip::udp::socket> { using type = boost::asio::ip::udp::endpoint; };

  template<typename S>
  using endpoint_type_from_socket_t = typename endpoint_type_from_socket<S>::type;

  template<typename S>
  using protocol_type_from_socket_t = endpoint_type_from_socket_t<S>::protocol_type;

 template<typename S>
  struct resolver_type_from_socket;

  template<>
  struct resolver_type_from_socket<boost::asio::ip::tcp::socket> { using type = boost::asio::ip::tcp::resolver; };

  template<>
  struct resolver_type_from_socket<boost::asio::ip::udp::socket> { using type = boost::asio::ip::udp::resolver; };

  template<typename S>
  using resolver_type_from_socket_t = typename resolver_type_from_socket<S>::type;
}


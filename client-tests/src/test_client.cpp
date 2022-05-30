#include <gtest/gtest.h>

#include "client.h"
#include <atomic>
#include <iostream>

using namespace cl;

port_t g_default_sender_port   = 5555;
port_t g_default_reciever_port = 5554;

auto g_default_sender_set_endpoints_event = event_set_endpoints(
    { .port=g_default_reciever_port,   .address={} },
    { .port=g_default_sender_port, .address={} }
);
auto g_default_reciever_set_endpoints_event = event_set_endpoints(
    { .port=g_default_sender_port,   .address={} },
    { .port=g_default_reciever_port, .address={} }
);


auto print(const auto& name, auto data) {
    std::cout << name << std::string(data.begin(), data.end()) << "\n";  
}


TEST(test_client, client_one_port) {

    client<udp_socket_t> reciever;
    client<udp_socket_t> sender;

    // todo: to look at resolution when only server port is known for now you must give port for both!
    // std::atomic<std::size_t> count_recieved{0};

    // reciever.dispatch(event_set_endpoints(),
    //                   event_set_on_recieve_handler([&count_recieved](data_t d) { std::cout << std::string(d.begin(), d.end()) << "\n"; count_recieved.fetch_add(1, std::memory_order_relaxed); }), 
    //                   event_start());
    
    // sender.dispatch(event_set_endpoints(),
    //                 event_set_on_recieve_handler([&count_recieved](data_t d) { std::cout << std::string(d.begin(), d.end()) << "\n"; count_recieved.fetch_add(1, std::memory_order_relaxed); }),
    //                 event_start());
                    
    // sender.dispatch(event_write_request(data_t(255, 0x32)), event_write_request(data_t(255, 0x32)), event_write_request(data_t(255, 0x32)));
    // reciever.dispatch(event_write_request(data_t(255, 0x31)), event_write_request(data_t(255, 0x31)), event_write_request(data_t(255, 0x31)));

    while(count_recieved.load() < 5) std::cout << count_recieved.load() << "\n";   

    std::cout << "end\n";  

}

TEST(test_client, client_two_ports) {

    client<udp_socket_t> reciever;
    client<udp_socket_t> sender;
    
    std::atomic<std::size_t> count_recieved{0};

    reciever.dispatch(g_default_reciever_set_endpoints_event,
                      event_set_on_recieve_handler([&count_recieved](data_t d) { std::cout << std::string(d.begin(), d.end()) << "\n"; count_recieved.fetch_add(1, std::memory_order_relaxed); }), 
                      event_start());
    
    sender.dispatch(g_default_sender_set_endpoints_event,
                    event_set_on_recieve_handler([&count_recieved](data_t d) { std::cout << std::string(d.begin(), d.end()) << "\n"; count_recieved.fetch_add(1, std::memory_order_relaxed); }),
                    event_start());
                    
    sender.dispatch(event_write_request(data_t(255, 0x32)), event_write_request(data_t(255, 0x32)), event_write_request(data_t(255, 0x32)));
    reciever.dispatch(event_write_request(data_t(255, 0x31)), event_write_request(data_t(255, 0x31)), event_write_request(data_t(255, 0x31)));

    while(count_recieved.load() < 5) std::cout << count_recieved.load() << "\n";   

    std::cout << "end\n";

}



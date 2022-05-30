// #include "client.h"

// #include <memory.h>






// namespace cl {


// struct event_set_endpoints {


//     event_set_endpoints(std::uint16_t p) 
//         : server(std::make_unique<endpoint>(p))
//         , client(std::make_unique<endpoint>(p))
//     { }

//     event_set_endpoints(std::uint16_t p, std::string a) 
//         : server(std::make_unique<endpoint>(p, a))
//         , client(std::make_unique<endpoint>(p, a))
//     { }

//     event_set_endpoints(endpoint se) 
//         : server(std::make_unique<endpoint>(std::move(se)))
//         , client(std::make_unique<endpoint>(server->port))
//     { }

//     event_set_endpoints(endpoint se, endpoint ce) 
//         : server(std::make_unique<endpoint>(std::move(se))) 
//         , client(std::make_unique<endpoint>(std::move(ce)))
//     { }

//     std::unique_ptr<endpoint> server;
//     std::unique_ptr<endpoint> client;
// };


// struct event_add_on_recieve_handler {
//     std::shared_ptr<on_recieve_handler_t> onrecv;

//     event_add_on_recieve_handler(on_recieve_handler_t h) 
//         : onrecv(std::make_shared<on_recieve_handler_t>(std::move(h)))
//     { }
// };

// struct state_awaiting_subscription {};
// struct state_idle {};
// struct state_ready {
//     std::shared_ptr<on_recieve_handler_t> onrecv;
//     state_ready(std::shared_ptr<on_recieve_handler_t> h) 
//         : onrecv(std::move(h))
//     { }
// };

// /** @brief defines all possible states and transitions */
// template<typename Socket>
// class client<Socket>::client_fsm_impl: public fsm::fsm<client_fsm_impl, state_t> {
// public:
//     template<typename State, typename Event>
//     auto on_event(State&, const Event&) {
//         // todo: need an error_code to wrap boost/std error_code -> for now throw for invalid transition
//         throw std::logic_error("Invalid transition");
//     }

//     auto on_event(state_idle&, const event_set_endpoints& e) -> optional_state_t {
//         //io_service_ = io_service_t();
//         socket_ = std::make_shared<client<Socket>::client_socket_t>(io_service_, *e.client);
//         return state_awaiting_subscription {};
//     }
//     ///auto on_event(state_awaiting_endpoints&, const event_set_endpoints& e) -> optional_state_t;
//     auto on_event(state_awaiting_subscription&, const event_add_on_recieve_handler& e) -> optional_state_t { return state_ready(std::move(e.onrecv));}

// private:
//     // the basic init flow is set, these dont have to be stored when set endpoints happen. Stash then in state unless fsm is to be the owner of io and sock, aka client does not take ownership
//     io_service_t io_service_;
//     std::shared_ptr<client_socket_t> socket_;

//     //auto on_event(state_connecting&, const event_connected&) -> optional_state_t;
//     //auto on_event(state_connecting& s, const event_timeout&) -> optional_state_t;
// };

// template<typename Socket>
// client<Socket>::client() : fsm_(std::make_shared<client_fsm_impl>()) {};

// // template<typename Socket>
// // template<typename... Args>
// // auto client<Socket>::set_endpoints(Args&& ... args) -> void {
// //     fsm_->dispatch(event_set_endpoints(std::forward<Args>(args) ...));
// //     return;
// // };

// // template<typename Socket>
// // auto client<Socket>::register_on_recieve_handler(on_recieve_handler_t h) -> void {
// //     //fsm_->dispatch(event_add_on_recieve_handler(std::forward<Args>(args) ...));
// //     return;
// // };


// // template<typename Socket>
// // auto client<Socket>::client_fsm_impl::on_event(state_idle&, const event_set_endpoints& e) -> optional_state_t {
// //     //io_service_ = io_service_t();
// //     socket_ = std::make_shared<client<Socket>::client_socket_t>(io_service_, *e.client);
// //     return state_awaiting_subscription {};
// // };

// // // template<typename Socket>
// // // auto client<Socket>::client_fsm_impl::on_event(state_awaiting_endpoints&, const event_set_endpoints& e) -> optional_state_t {
// // //     io_service_ = std::make_shared(io_service_t{})
// // //     socket_ = std::make_shared(client<Socket>::client_socket_t(*io_service_, *e.client))
// // //     return state_ready {};
// // // };

// // template<typename Socket>
// // auto client<Socket>::client_fsm_impl::on_event(state_awaiting_subscription&, const event_add_on_recieve_handler& e) -> optional_state_t {
// //     // stash the handler we have not created the connectioon yet
// //     return state_ready(std::move(e.onrecv));
// // };


// template class client<udp_socket_t>;

// }
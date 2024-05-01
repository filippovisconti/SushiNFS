/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>
#include <grpc/grpc_usb.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "build/HelloWorld.grpc.pb.h"
#endif
#include <sys/socket.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;
#include <grpcpp/grpcpp.h>

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {
	Status SayHello(ServerContext *context, const HelloRequest *request, HelloReply *reply) override
	{
		std::string prefix("Hello ");
		reply->set_message(prefix + request->name());
		return Status::OK;
	}
};

void RunServer()
{
	// std::string server_address("0.0.0.0:50051");
	GreeterServiceImpl service;
	int VID = 0x1234; // Replace with VID
	int PID = 0x5678; // Replace with PID

	ServerBuilder builder;
	// builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	std::unique_ptr<Server> server(builder.BuildAndStart());
	grpc_completion_queue *cq = grpc_completion_queue_create_for_next(nullptr);
	grpc_server_register_completion_queue(server->c_server(), cq, nullptr);
	grpc_channel *channel = grpc_server_add_insecure_channel_from_usb(
		server->c_server(), nullptr, VID, PID); // Replace with your VID and PID

	// std::cout << "Server listening on " << server_address << std::endl;
	std::cout << std::endl;

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever return.
	// server->Wait();
	GPR_ASSERT(channel != nullptr);
	builder.RegisterService(&service);

	std::cout << "Server started..." << std::endl;
	// Handle a maximum of 10 requests
	int num_requests_handled = 0;
	while (num_requests_handled < 10) {
		grpc_event event;
		grpc_completion_queue_next(cq, gpr_inf_future(GPR_CLOCK_REALTIME), &event);
		// Handle events if needed

		// Increment the counter for each request handled
		num_requests_handled++;
	}

	// Shutdown the server
	grpc_server_shutdown_and_notify(server->c_server(), cq, nullptr);
	grpc_server_destroy(server->c_server());
	grpc_completion_queue_destroy(cq);
	std::cout << "Server shutdown" << std::endl;
}

int main(int argc, char **argv)
{
	RunServer();

	return 0;
}

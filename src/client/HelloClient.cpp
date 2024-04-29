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
#include <sys/socket.h>
#include <netinet/in.h>

#include <grpc++/grpc++.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "build/HelloWorld.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

class GreeterClient {
    public:
	GreeterClient(std::shared_ptr<Channel> channel)
		: stub_(Greeter::NewStub(channel))
	{
	}

	// Assembles the client's payload, sends it and presents the response back
	// from the server.
	std::string SayHello(const std::string &user)
	{
		// Data we are sending to the server.
		HelloRequest request;
		request.set_name(user);

		// Container for the data we expect from the server.
		HelloReply reply;

		// Context for the client. It could be used to convey extra information to
		// the server and/or tweak certain RPC behaviors.
		ClientContext context;

		// The actual RPC.
		Status status = stub_->SayHello(&context, request, &reply);

		// Act upon its status.
		if (status.ok()) {
			return reply.message();
		} else {
			std::cout << status.error_code() << ": " << status.error_message() << std::endl;
			return "RPC failed";
		}
	}

    private:
	std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char **argv)
{
	// Instantiate the client. It requires a channel, out of which the actual RPCs
	// are created. This channel models a connection to an endpoint (in this case,
	// localhost at port 50051). We indicate that the channel isn't authenticated
	// (use of InsecureChannelCredentials()).
	// GreeterClient greeter(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
	int socketFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(50051);
	int st = connect(socketFd, (struct sockaddr *)&address, sizeof(address));
	std::cout << "st " << st << std::endl;
	GreeterClient greeter(grpc::CreateInsecureChannelFromFd("localhost", socketFd));
	std::string user("world");
	std::string reply = greeter.SayHello(user);
	std::cout << "Greeter received: " << reply << std::endl;

	return 0;
}

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
#include <thread>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h> /* For O_RDWR */
#include <unistd.h> /* For open(), creat() */
#include <linux/usb/ch9.h>
#include <grpc++/grpc++.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "build/HelloWorld.grpc.pb.h"
#endif
#include <grpc/grpc_usb.h>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel_posix.h>
extern "C" {
#include "gadget.h"
}
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
		std::cout << "context" << std::endl;

		// The actual RPC.
		try {
			Status status = stub_->SayHello(&context, request, &reply);
			std::cout << "status" << std::endl;
			// Act upon its status.
			if (status.ok()) {
				std::cout << "ok" << std::endl;
				return reply.message();
			} else {
				std::cout << status.error_code() << ": " << status.error_message()
					  << std::endl;
				return "RPC failed";
			}

		} catch (const std::exception &e) {
			std::cerr << e.what() << '\n';
			return "RPC failed";
		}
	}

    private:
	std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char **argv)
{
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <VID> <PID>" << std::endl;
		return 1;
	}
	int VID = int(strtol(argv[1], NULL, 16));
	int PID = int(strtol(argv[2], NULL, 16));
	std::cout << "VID: " << VID << ", PID: " << PID << std::endl;
	grpc_init();

	const char target[] = "usb-client";
	int fd = usb_raw_open();
	const char *device = "1000480000.usb";
	const char *driver = "1000480000.usb";
	usb_raw_init(fd, USB_SPEED_HIGH, driver, device);
	usb_raw_run(fd);

	// grpc_channel *usb_client_channel = grpc_insecure_channel_create_from_usb(target, VID, PID, nullptr);
	std::shared_ptr<grpc::Channel> fd_channel = grpc::CreateInsecureChannelFromFd(target, fd);
	GPR_ASSERT(fd_channel != nullptr);
	std::cout << "USB channel created" << std::endl;
	// run ep0_loop in a separate thread
	std::thread ep0_thread(ep0_loop, fd);
	ep0_thread.detach();
	std::cout << "USB raw open" << fd << std::endl;
	// GPR_ASSERT(usb_client_channel != nullptr);

	// std::shared_ptr<Channel> ch = grpc::CreateChannelInternal(
	// 	"usb-client", usb_client_channel,
	// 	std::vector<std::unique_ptr<grpc::experimental::ClientInterceptorFactoryInterface> >());
	std::cout << "Internal channel created" << std::endl;
	GreeterClient greeter(fd_channel);

	std::cout << "Greeter client created" << std::endl;

	std::string user("world");
	std::string reply = greeter.SayHello(user);
	std::cout << "Greeter received: " << reply << std::endl;
	ep0_thread.join();
	close(fd);
	return 0;
}

#include <iostream>
#include <stdint.h>
#include <vector>
#include <cmath>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

using namespace std;
using uchar = unsigned char;
using vecmat = std::vector<std::vector<float>> ;

#define OUT(__o__) std::cout<< __o__ << std::endl;

float dx = 0, dy = 0, length = 200, increment = 10;

float char_to_float(char* in) // necessary for the feedback
{
	float out;
	char *trf;
	trf = (char*)&out;
	*trf++ = in[3];
	*trf++ = in[2];
	*trf++ = in[1];
	*trf = in[0];
	return(out);
}

vecmat get_cmd_matrix()
{
	//CALIBRATION POINT (nice pcd pont : 500.61 -162.87 -13.07 0.72 179.29 -39.67)
	//-------------------------------
	float x = 500.61, y = -162.87, z = -400.72;
	//-------------------------------
	float row = 0.82, pitch = 178.93, yaw = -1.82, velo = 10;;

	vecmat cmd_mat;
	std::vector<float> cmd;

	cmd.push_back(velo); 
	cmd.push_back(x); 
	cmd.push_back(y); 
	cmd.push_back(z); 
	cmd.push_back(row); 
	cmd.push_back(pitch); 
	cmd.push_back(yaw);

	cmd_mat.push_back(cmd);
	cmd.clear();

	return cmd_mat;
}

void run_trajectory(std::string&& ip_address)
{
	vecmat cmd_matrix = get_cmd_matrix();
	char *sendbuf;
	float velo, pose1, pose2, pose3,\
		  pose4, pose5, pose6;
	constexpr auto DEFAULT_BUFLEN = 512;

	int sockfd; //ID for socket
	struct sockaddr_in servaddr, cliaddr; //address-structure for IP-Address

	uint16_t DEFAULT_PORT = 5000;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	//create socket
	servaddr.sin_family = AF_INET; //for protocol family
	servaddr.sin_addr.s_addr = inet_addr(ip_address.c_str()); //controller's IP-Address
	servaddr.sin_port = htons(DEFAULT_PORT); //port to be used

	char recvbuf[DEFAULT_BUFLEN];  //buffer for response
	int n = 0;
	float x = 0, y = 0, z = 0, r = 0, p = 0, yaw = 0;
	/*
		yes_con will be -1 if the connection fails per unix man page
	*/
	int yes_con = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)); //connect to controller
	uint16_t fail = -1;


	for (int i = 0; i < cmd_matrix.size(); ++i)
	{
		while (true)
		{
			velo = cmd_matrix[i][0];
			pose1 = cmd_matrix[i][1];
			pose2 = cmd_matrix[i][2];
			pose3 = cmd_matrix[i][3];
			pose4 = cmd_matrix[i][4];
			pose5 = cmd_matrix[i][5];
			pose6 = cmd_matrix[i][6];

			OUT("yes_con: "  << ((yes_con > fail) ? yes_con : fail) );

			char* velos = (char*)&velo;
			char* pose1s = (char*)&pose1;
			char* pose2s = (char*)&pose2;
			char* pose3s = (char*)&pose3;
			char* pose4s = (char*)&pose4;
			char* pose5s = (char*)&pose5;
			char* pose6s = (char*)&pose6;

			char command[] = "TX90servabcdabcdabcdabcdabcdabcdabcd";
			command[8] = (velos[3] == 0) ? ' ' : velos[3];
			command[9] = (velos[2] == 0) ? ' ' : velos[2];
			command[10] = (velos[1] == 0) ? ' ' : velos[1];
			command[11] = (velos[0] == 0) ? ' ' : velos[0];

			command[12] = (pose1s[3] == 0) ? ' ' : pose1s[3];
			command[13] = (pose1s[2] == 0) ? ' ' : pose1s[2];
			command[14] = (pose1s[1] == 0) ? ' ' : pose1s[1];
			command[15] = (pose1s[0] == 0) ? ' ' : pose1s[0];

			command[16] = (pose2s[3] == 0) ? ' ' : pose2s[3];
			command[17] = (pose2s[2] == 0) ? ' ' : pose2s[2];
			command[18] = (pose2s[1] == 0) ? ' ' : pose2s[1];
			command[19] = (pose2s[0] == 0) ? ' ' : pose2s[0];

			command[20] = (pose3s[3] == 0) ? ' ' : pose3s[3];
			command[21] = (pose3s[2] == 0) ? ' ' : pose3s[2];
			command[22] = (pose3s[1] == 0) ? ' ' : pose3s[1];
			command[23] = (pose3s[0] == 0) ? ' ' : pose3s[0];

			command[24] = (pose4s[3] == 0) ? ' ' : pose4s[3];
			command[25] = (pose4s[2] == 0) ? ' ' : pose4s[2];
			command[26] = (pose4s[1] == 0) ? ' ' : pose4s[1];
			command[27] = (pose4s[0] == 0) ? ' ' : pose4s[0];

			command[28] = (pose5s[3] == 0) ? ' ' : pose5s[3];
			command[29] = (pose5s[2] == 0) ? ' ' : pose5s[2];
			command[30] = (pose5s[1] == 0) ? ' ' : pose5s[1];
			command[31] = (pose5s[0] == 0) ? ' ' : pose5s[0];

			command[32] = (pose6s[3] == 0) ? ' ' : pose6s[3];
			command[33] = (pose6s[2] == 0) ? ' ' : pose6s[2];
			command[34] = (pose6s[1] == 0) ? ' ' : pose6s[1];
			command[35] = (pose6s[0] == 0) ? ' ' : pose6s[0];

			sendbuf = (char *)malloc(strlen(command) + 1);

			strcpy(sendbuf, command);

			OUT("sendbuf: " << sendbuf)

			OUT("sockfd: " << sockfd);
			
			/*
			ssize_t send(int sockfd, const void *buf, size_t len, int flags);


			flags          
			MSG_DONTROUTE
              Don't use a gateway to send out the packet, send to  hosts  only
              on  directly  connected  networks.  This is usually used only by
              diagnostic or routing programs.  This is defined only for proto‐
              col families that route; packet sockets don't.

			MSG_DONTWAIT (since Linux 2.2)
              Enables  nonblocking  operation;  if  the operation would block,
              EAGAIN or EWOULDBLOCK is returned  (this  can  also  be  enabled
              using the O_NONBLOCK flag with the F_SETFL fcntl(2)).



			*/
			ssize_t send_outcome = send(sockfd, sendbuf, sizeof(sendbuf), MSG_DONTROUTE);
			// sendto(sockfd, sendbuf, sizeof(sendbuf), MSG_DONTWAIT, nullptr, 0); //same as send command
			OUT("send_outcome: " << send_outcome);

			usleep(5*10000); //simply wait x ms for controller to answer - replace this with an event-driven approach if needed
		   	n = read(sockfd, recvbuf, 1000); //read socket/received data || ,0,NULL,NULL
		   	recvbuf[n] = 0; //add string termination character 
		   	// fputs(recvbuf, stdout);

			if (n > 0)
			{
			   	OUT("Received " << n << " Bytes in response: ");

				char feedback[4] = {0}; 
				feedback[0] = (recvbuf[12] == 0) ? ' ' : recvbuf[12];
				feedback[1] = (recvbuf[13] == 0) ? ' ' : recvbuf[13];
				feedback[2] = (recvbuf[14] == 0) ? ' ' : recvbuf[14];
				feedback[3] = (recvbuf[15] == 0) ? ' ' : recvbuf[15];

				x = char_to_float(feedback);

				feedback[0] = (recvbuf[16] == 0) ? ' ' : recvbuf[16];
				feedback[1] = (recvbuf[17] == 0) ? ' ' : recvbuf[17];
				feedback[2] = (recvbuf[18] == 0) ? ' ' : recvbuf[18];
				feedback[3] = (recvbuf[19] == 0) ? ' ' : recvbuf[19];

				y = char_to_float(feedback);

				feedback[0] = (recvbuf[20] == 0) ? ' ' : recvbuf[20];
				feedback[1] = (recvbuf[21] == 0) ? ' ' : recvbuf[21];
				feedback[2] = (recvbuf[22] == 0) ? ' ' : recvbuf[22];
				feedback[3] = (recvbuf[23] == 0) ? ' ' : recvbuf[23];

				z = char_to_float(feedback);

				feedback[0] = (recvbuf[24] == 0) ? ' ' : recvbuf[24];
				feedback[1] = (recvbuf[25] == 0) ? ' ' : recvbuf[25];
				feedback[2] = (recvbuf[26] == 0) ? ' ' : recvbuf[26];
				feedback[3] = (recvbuf[27] == 0) ? ' ' : recvbuf[27];

				r = char_to_float(feedback);

				feedback[0] = (recvbuf[28] == 0) ? ' ' : recvbuf[28];
				feedback[1] = (recvbuf[29] == 0) ? ' ' : recvbuf[29];
				feedback[2] = (recvbuf[30] == 0) ? ' ' : recvbuf[30];
				feedback[3] = (recvbuf[31] == 0) ? ' ' : recvbuf[31];

				p = char_to_float(feedback);

				feedback[0] = (recvbuf[32] == 0) ? ' ' : recvbuf[32];
				feedback[1] = (recvbuf[33] == 0) ? ' ' : recvbuf[33];
				feedback[2] = (recvbuf[34] == 0) ? ' ' : recvbuf[34];
				feedback[3] = (recvbuf[35] == 0) ? ' ' : recvbuf[35];

				yaw = char_to_float(feedback);

				usleep(10000);; //This is necessary

				int i;
				for (i=0; i<n+1; i++) printf("%02X ", (unsigned char)recvbuf[i]); //output every byte
				printf("\n");
				int ErrorCode = (unsigned char)recvbuf[17];
				if (ErrorCode == 128) printf("Error: Controller is not in external trigger mode, error condition was set to 0x%02X", ErrorCode);
				close(sockfd);
			}

			if (std::fabs(x - pose1) + std::fabs(y - pose2) + std::fabs(z - pose3) < 5)
			{
				break;
			}
			usleep(10000);
		}
	}
}

int main(int argc, char *argv[])
{
	std::string ip_address;
	if (argc != 2) // Validate the parameters
	{
		printf("usage: %s server-name\n", argv[0]);
		return EXIT_FAILURE;
	}
	else
		std::string ip_address = argv[0];

	OUT("running algorithm");

	run_trajectory(std::move(ip_address));

	usleep(5*10000);

	OUT("End connection");
	
	return EXIT_SUCCESS;
}
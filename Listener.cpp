//
// Project: dsn
//
//
//

#include "Listener.hpp"

Listener::Listener() { }

Listener::Listener(const int server_port) { server_port_ = server_port; }


// Helper

bool valid_reply_form(const std::string & response) {
  // >..._
  std::cout << "first: " << response.substr(0,1) << "." << std::endl;
  std::cout << "second:" << response.substr(response.length()-1, 1) << "." << std::endl;
  return (response.substr(0,1) == ">" && response.substr(response.length()-1, 1) == "_");
}


bool valid_transmit_form(const std::string & response) {
  // One of three choices representing a stage
  // >...&    first block in transmission
  // &...&    second block and onwards in transmission
  // &..._    last block in transmission
  // transmit form must satisfy one of these

  if (response == ">badblock_")
    return false;

  std::cout << "[Client] first: " << response.substr(0, 1) << "." << std::endl;
  std::cout << "[Client] second: " << response.substr(response.length()-1, 1) << "." << std::endl;

  bool one_chunk = (response.substr(0,1) == ">" && response.substr(response.size()-1, 1) == "_");
  bool first_chunk = (response.substr(0,1) == ">" && response.substr(response.size()-1, 1) == "&");
  bool chunk_n = (response.substr(0,1) == "&" && response.substr(response.size()-1, 1) == "&");
  bool last_chunk = (response.substr(0,1) == "&" && response.substr(response.size()-1, 1) == "_");

  std::cout << "[Client] transmit form [" << response << "]: " 
            << ((one_chunk || first_chunk || chunk_n || last_chunk) ? "good" : "bad") << std::endl;

  return (one_chunk || first_chunk || chunk_n || last_chunk);
}

void Listener::run() {

  std::cout << "[Listener] Initializing server..." << std::endl;

  int server_fd, sock, value;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  
  const int server_buf_size = 100;
  char server_buf[server_buf_size] = {0};
  int client_buf_size = 100;

  // Create socket file descriptor
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Attach socket to given port
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(server_port_);
  
  // bind socket to port
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  std::cout << "[Listener] Waiting for requests..." << std::endl;

  // listen on port
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  std::string response = "";

  while (true) {
    // accept connections
    sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (sock < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    while (true) {
      // clear buffer before every use
      for (char & ch : server_buf)
        ch = 0;
 
      value = recv(sock, server_buf, server_buf_size, 0);
      if (value == 0) {
        std::cout << "[Listener] Remote host terminated connection. Exiting..." << std::endl;
        exit(0);
      }

      std::cout << "[Listener] Message [" << value << "]: " << server_buf << std::endl;
      response = server_buf;
      response = response.substr(0, value);

      if (response.substr(0,1) == "$") {
        client_buf_size = std::stoi(response.substr(1, response.find('_')-1));
        
  
        std::string buffer_size_message = std::string(">") + std::to_string(server_buf_size) + "_";
        const char * size_message = buffer_size_message.c_str();
   
        std::cout << "[Listener] Sending buffer size message to node: " << buffer_size_message << std::endl;
  
        send(sock, size_message, strlen(size_message), 0);
      }
  
      if (response.substr(0,1) == "?") {
        std::string requested = response.substr(1, response.find('_')-1);
        std::cout << "[Listener] Requested block name/hash: " << requested << std::endl;
        std::string filename = "./" + requested;
        std::ifstream file(filename, std::ios::binary);
  
        if (!file.good()) {
          std::cout << "[Listener] Requested block not found." << std::endl;       
          char * not_found_message = ">badblock_";
          send(sock, not_found_message, strlen(not_found_message), 0);
          return;
        }
    
        std::string data = "";
        if (!file.is_open()) {
          std::cout << "[Listener] Error opening file: " << requested << std::endl;
        } else {
          std::string current = "";
          std::string data = "";
          // Do not ignore whitespace
          while (std::getline(file, current))
           data  = data + current + '\n';
  
          if (data.length()+1 > client_buf_size) {
            // Split block into chunks of client buffer size (minus two for the >..._ reply form and 
            // terminating underscore)
            std::vector<std::string> chunks;
            chunks.reserve(data.size()/client_buf_size-2);
            int chunk_size = client_buf_size-2;
            for (int i = 0; i < data.length(); i += chunk_size) {
              chunks.push_back(data.substr(i, chunk_size));
            }


            // Transmission
            // Conform to reply format
            for (int i = 0; i < chunks.size(); ++i) {
              if (i == 0)
                chunks[0] = std::string(">") + chunks[0] + "&";
              // last element
              else if (i == chunks.size()-1)
                chunks[i] = std::string("&") + chunks[i] + "_";
              else
                chunks[i] = std::string("&") + chunks[i] + "&";
            }
            // for debug
            std::cout << "Data: " << data << std::endl;
            for (std::string & chunk : chunks)
              std::cout << "\tChunk: " << chunk << std::endl;

            // send
            for (std::string & chunk : chunks) {
              send(sock, chunk.c_str(), strlen(chunk.c_str()), 0);
            }

          } else {
            // Send message in single transmission
            std::string block = std::string(">") + data + "_";
            block = block.substr(0, client_buf_size);
            std::cout << "[Listener] Sending data:" << block << std::endl; 
            send(sock, block.c_str(), strlen(block.c_str()), 0);
          }
        }
  
        file.close();
  
      }
    }
  
  }

}



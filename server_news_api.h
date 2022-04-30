#pragma once
#include <string>
#include <vector>

#include "utils.h"
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFFacilities.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFServer.h"

class ServerNewsApi {
 private:
  static WFFacilities::WaitGroup global_wait_group_;
  const static std::vector<std::string> news_classe_names;
  static void StopWaitGroup(int signal);
  static void Process(WFHttpTask* server_task);

  struct RequestData {
    std::vector<HrefItem>& items;
    size_t idx;
  };
  static void* RequestEvaluation(void *data);

  int port_;
  static constexpr float threadhold_negative_evaluation_ = -0.5;
  // static constexpr unsigned int request_threads_num = 100;

 public:
  void Run(int port);
  ServerNewsApi() {}
  ~ServerNewsApi() {}
};

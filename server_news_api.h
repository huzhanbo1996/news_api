
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFFacilities.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFServer.h"

#include<vector>
#include<string>

class ServerNewsApi {
 private:
  static WFFacilities::WaitGroup global_wait_group_;
  const static std::vector<std::string> news_classe_names;
  static void StopWaitGroup(int signal);
  static void Process(WFHttpTask *server_task);

  int port_;
  static constexpr float threadhold_negative_evaluation_ = -0.5;

 public:
  void Run(int port);
  ServerNewsApi() {}
  ~ServerNewsApi() {}
};

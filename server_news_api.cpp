
#include "server_news_api.h"

#include <signal.h>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "utils.h"

WFFacilities::WaitGroup ServerNewsApi::global_wait_group_(1);

const std::vector<std::string> ServerNewsApi::news_classe_names{
    "hotnews", "ulist focuslistnews", "ulist"};

void ServerNewsApi::Run(int port) {
  port_ = port;

  signal(SIGINT, &StopWaitGroup);

  WFHttpServer server(Process);
  // port = atoi(argv[1]);
  if (server.start(port) == 0) {
    global_wait_group_.wait();
    server.stop();
  } else {
    perror("Cannot start server");
    exit(1);
  }
}

void ServerNewsApi::StopWaitGroup(int signal) { global_wait_group_.done(); }

void* ServerNewsApi::RequestEvaluation(void *data)
{
  RequestData* request_data = (RequestData *)data;
  auto &news_with_link = request_data->items[request_data->idx];
  const auto &evaluation_json =
      Utils::HttpReqSync("http://baobianapi.pullword.com:9091/get.php",
                          "POST", news_with_link.title.c_str());
  rapidjson::Document document;
  document.Parse(evaluation_json.c_str());
  if (document.IsObject() && document.HasMember("result") &&
      document["result"].IsFloat()) {
    news_with_link.positive_evaluation = document["result"].GetFloat();
  }
  return nullptr;
}
void ServerNewsApi::Process(WFHttpTask *server_task) {
  protocol::HttpRequest *req = server_task->get_req();
  protocol::HttpResponse *resp = server_task->get_resp();
  long long seq = server_task->get_task_seq();

  // get news page html
  const auto &html = Utils::HttpReqSync("http://news.baidu.com/", "GET", "");

  // find all news with their link
  auto news_with_link_vec = Utils::FindItemUnderClass(html, news_classe_names);

  // get their positive evaluation via baobianapi.pullword.com
  RequestData data{news_with_link_vec, 0};
  std::vector<pthread_t> threads_id(news_with_link_vec.size());
  for (size_t i = 0; i < news_with_link_vec.size(); ++i) {
    pthread_t threadId;
    pthread_create(&threadId, NULL, &RequestEvaluation, &data);
    threads_id[i] = threadId;
    data.idx++;
  }

  for (size_t i = 0; i < news_with_link_vec.size(); ++i) {
    pthread_join(threads_id[i], NULL);
  }
  
  // make body json
  rapidjson::Document response_document;
  response_document.SetArray();
  auto &allocator = response_document.GetAllocator();
  for (const auto &news_with_link : news_with_link_vec) {
    if (news_with_link.positive_evaluation > threadhold_negative_evaluation_) {
      continue;
    }
    rapidjson::Value news_item(rapidjson::kObjectType);
    news_item.AddMember(
        "title",
        rapidjson::Value(rapidjson::kStringType)
            .SetString(rapidjson::StringRef(news_with_link.title.c_str())),
        allocator);
    news_item.AddMember(
        "link",
        rapidjson::Value(rapidjson::kStringType)
            .SetString(rapidjson::StringRef(news_with_link.url.c_str())),
        allocator);
    news_item.AddMember("evaluation",
                        rapidjson::Value(rapidjson::kNumberType)
                            .SetFloat(news_with_link.positive_evaluation),
                        allocator);
    response_document.PushBack(news_item.Move(), allocator);
  }

  rapidjson::StringBuffer response_buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(response_buffer);
  response_document.Accept(writer);

  // append body, it's ok, the string always ends with \0
  resp->append_output_body(response_buffer.GetString());

  // make header
  resp->set_http_version("HTTP/1.1");
  resp->set_status_code("200");
  resp->set_reason_phrase("OK");
  resp->add_header_pair("Content-Type", "text/html");
  resp->add_header_pair("Server", "Luc's server");

  // no more than 10 requests on the same connection
  if (seq == 9) resp->add_header_pair("Connection", "close");

}

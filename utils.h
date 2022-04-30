#pragma once
#include <string.h>

#include <atomic>
#include <string>
#include <vector>

#include "gumbo.h"
#include "workflow/WFTaskFactory.h"

struct HrefItem {
  std::string url;
  std::string title;
  float positive_evaluation;
};

class Utils {
 private:
  struct HttpReqUserData {
    std::atomic_bool callback_ready_;
    std::string req_response_;
    HttpReqUserData(bool callback_ready, const std::string& req_response)
        : callback_ready_(callback_ready), req_response_(req_response) {}
  };

  static void HttpReqCallback(WFHttpTask* task);
  static void SearchForLinks(GumboNode* node,
                             std::vector<GumboNode*>& all_nodes_with_links);
  static void SearchForClasses(GumboNode* node, const char* cls_name,
                               std::vector<GumboNode*>& all_node_with_class);
  static void SearchForText(GumboNode* node,
                            std::vector<GumboNode*>& all_node_with_text);

 public:
  static std::vector<HrefItem> FindItemUnderClass(
      const std::string& html, const std::vector<std::string>& class_names);
  static std::string HttpReqSync(const std::string& url, const std::string type,
                                 const std::string& msg = "");

  Utils();
  ~Utils();
};
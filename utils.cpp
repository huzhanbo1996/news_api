
#include "utils.h"

#include <atomic>
#include <numeric>

#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFFacilities.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFServer.h"

Utils::Utils() {}

Utils::~Utils() {}

// TODO: combine these 3 func
// from gumbo examples, TODO: make recursion to iteration
void Utils::SearchForLinks(GumboNode* node,
                           std::vector<GumboNode*>& all_nodes_with_links) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }

  GumboAttribute* href;
  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
    all_nodes_with_links.push_back(node);
    return;
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    SearchForLinks(static_cast<GumboNode*>(children->data[i]),
                   all_nodes_with_links);
  }
  return;
}

// from gumbo examples, TODO: make recursion to iteration, use std::string
void Utils::SearchForClasses(GumboNode* node, const char* cls_name,
                             std::vector<GumboNode*>& all_node_with_class) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* cls_attr;
  if ((cls_attr = gumbo_get_attribute(&node->v.element.attributes, "class")) &&
      strstr(cls_attr->value, cls_name) != NULL) {
    all_node_with_class.push_back(node);
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    SearchForClasses(static_cast<GumboNode*>(children->data[i]), cls_name,
                     all_node_with_class);
  }
  return;
}

void Utils::SearchForText(GumboNode* node,
                          std::vector<GumboNode*>& all_node_with_text) {
  if (node->type == GUMBO_NODE_TEXT) {
    all_node_with_text.push_back(node);
  } else {
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      SearchForText(static_cast<GumboNode*>(children->data[i]),
                    all_node_with_text);
    }
  }

  return;
}

// TODO: use c++ style iterator
std::vector<HrefItem> Utils::FindItemUnderClass(
    const std::string& html, const std::vector<std::string>& class_names) {
  std::vector<HrefItem> href_items;
  GumboOutput* news_output = gumbo_parse(html.c_str());

  for (const auto& new_class_name : class_names) {
    // for each class name
    std::vector<GumboNode*> all_node_with_class;
    SearchForClasses(news_output->root, new_class_name.c_str(),
                     all_node_with_class);

    // for each node with target class tag
    for (const auto& news_node : all_node_with_class) {
      std::vector<GumboNode*> all_news_nodes_with_links;
      SearchForLinks(news_node, all_news_nodes_with_links);

      // for each sub nodes containing news' link and title
      for (unsigned int i = 0; i < all_news_nodes_with_links.size(); i++) {
        GumboNode* href_node = all_news_nodes_with_links[i];
        GumboAttribute* href_attr =
            gumbo_get_attribute(&href_node->v.element.attributes, "href");
        std::string href_url = std::string(href_attr->value);

        std::vector<GumboNode*> node_containing_text;
        SearchForText(href_node, node_containing_text);

        std::string href_text = std::accumulate(
            node_containing_text.begin(), node_containing_text.end(),
            std::string(""), [](std::string prev, GumboNode* node) {
              return prev + std::string(node->v.text.text);
            });

        href_items.push_back(HrefItem{href_url, href_text, 0.0});
      }
    }
  }

  gumbo_destroy_output(&kGumboDefaultOptions, news_output);
  return href_items;
}

void Utils::HttpReqCallback(WFHttpTask* task) {
  const auto& user_data = static_cast<HttpReqUserData*>(task->user_data);
  protocol::HttpRequest* req = task->get_req();
  protocol::HttpResponse* resp = task->get_resp();
  int state = task->get_state();
  int error = task->get_error();

  switch (state) {
    case WFT_STATE_SYS_ERROR:
      fprintf(stderr, "system error: %s\n", strerror(error));
      break;
    case WFT_STATE_DNS_ERROR:
      fprintf(stderr, "DNS error: %s\n", gai_strerror(error));
      break;
    case WFT_STATE_SSL_ERROR:
      fprintf(stderr, "SSL error: %d\n", error);
      break;
    case WFT_STATE_TASK_ERROR:
      fprintf(stderr, "Task error: %d\n", error);
      break;
    case WFT_STATE_SUCCESS:
      break;
  }

  if (state != WFT_STATE_SUCCESS) {
    fprintf(stderr, "Http request failed\n");
    user_data->callback_ready_ = true;
    return;
  }

  const void* body;
  size_t body_len;

  resp->get_parsed_body(&body, &body_len);
  user_data->req_response_ = std::string((const char*)body, body_len);
  user_data->callback_ready_ = true;
  return;
}

std::string Utils::HttpReqSync(const std::string& url, const std::string type,
                               const std::string& msg) {
#define REDIRECT_MAX 5
#define RETRY_MAX 2
  HttpReqUserData user_data{false, ""};
  WFHttpTask* task = WFTaskFactory::create_http_task(
      url, REDIRECT_MAX, RETRY_MAX, HttpReqCallback);
  task->user_data = &user_data;
  protocol::HttpRequest* req = task->get_req();
  req->add_header_pair("Accept", "*/*");
  req->add_header_pair("User-Agent", "Wget/1.14 (linux-gnu)");
  req->add_header_pair("Connection", "close");
  req->set_method(type);
  if (msg.length() > 0) {
    req->append_output_body(msg.c_str(), msg.size());
  }
  task->start();
  while (!user_data.callback_ready_) {
    ;
  }
  return user_data.req_response_;
}
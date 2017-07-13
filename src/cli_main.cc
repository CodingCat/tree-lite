/*!
 * Copyright 2017 by Contributors
 * \file cli_main.cc
 * \brief The command line interface program of treelite.
 *  This file is not included in dynamic library.
 */

#include <treelite/parser.h>
#include <dmlc/config.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <queue>
#include <string>

namespace treelite {

struct CLIParam : public dmlc::Parameter<CLIParam> {
  /*! \brief model format */
  std::string format;
  /*! \brief model file */
  std::string model_in;
  /*! \brief all the configurations */
  std::vector<std::pair<std::string, std::string> > cfg;

  // declare parameters
  DMLC_DECLARE_PARAMETER(CLIParam) {
    DMLC_DECLARE_FIELD(format).describe("Model format");
    DMLC_DECLARE_FIELD(model_in).describe("Input model path");
  }
  // customized configure function of CLIParam
  inline void Configure(const std::vector<std::pair<std::string, std::string> >& cfg) {
    this->cfg = cfg;
    this->InitAllowUnknown(cfg);
  }
};

DMLC_REGISTER_PARAMETER(CLIParam);

const char* PrintOp(Tree::Operator op) {
  switch(op) {
    case Tree::Operator::kEQ: return "==";
    case Tree::Operator::kLT: return "<";
    case Tree::Operator::kLE: return "<=";
    case Tree::Operator::kGT: return ">";
    case Tree::Operator::kGE: return ">=";
    default: return "";
  }
}

int CLIRunTask(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: <config>\n");
    return 0;
  }

  std::vector<std::pair<std::string, std::string> > cfg;
  cfg.push_back(std::make_pair("seed", "0"));

  std::ifstream cfgfile(argv[1], std::ifstream::in);
  dmlc::Config itr(cfgfile);
  for (const auto& entry : itr) {
    cfg.push_back(std::make_pair(entry.first, entry.second));
  }
  cfgfile.close();

  for (int i = 2; i < argc; ++i) {
    char name[256], val[256];
    if (sscanf(argv[i], "%[^=]=%s", name, val) == 2) {
      cfg.push_back(std::make_pair(std::string(name), std::string(val)));
    }
  }
  CLIParam param;
  param.Configure(cfg);

  std::unique_ptr<Parser> parser(Parser::Create(param.format));
  std::unique_ptr<dmlc::Stream> fi(dmlc::Stream::Create(param.model_in.c_str(), "r"));
  parser->Load(fi.get());

  std::vector<Tree> model = parser->Export();
  LOG(INFO) << "model size = " << model.size();

  std::ostringstream ss;
  ss << "\n";
  int i = 0;
  for (const auto& tree : model) {
    int nleaf = 0;
    ss << "Tree #" << i << "\n";
    std::queue<int> Q;
    Q.push(0);
    while (!Q.empty()) {
      const int nid = Q.front(); Q.pop();
      const Tree::Node& node = tree[nid];
      if (node.is_leaf()) {
        ss << "  " << nid << ": leaf_value=" << node.leaf_value() << ", parent=" << node.parent()
           << "\n";
        ++nleaf;
      } else {
        ss << "  " << nid << ": split_index=" << node.split_index() << ", threshold="
           << node.threshold() << ", op=" << PrintOp(node.comparison_op())
           << ", cleft=" << node.cleft() << ", cright=" << node.cright()
           << ", cdefault=" << node.cdefault();
        if (!node.is_root()) {
          ss << ", parent=" << node.parent() << "\n";
        } else {
          ss << "\n";
        }
        Q.push(node.cleft());
        Q.push(node.cright());
      }
    }
    ss << "Tree #" << i++ << " has " << nleaf << " leaves total\n\n";
  }
  LOG(INFO) << ss.str();

  return 0;
}

}  // namespace treelite

int main(int argc, char* argv[]) {
  return treelite::CLIRunTask(argc, argv);
}

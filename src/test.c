#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#define STR1(x) #x
#define STR(x) STR1(x)

#define ERROR_PREF __FILE__ ":" STR(__LINE__) ": "
#define ERROR(E,X,...) { \
  fprintf(stderr, ERROR_PREF X, ##__VA_ARGS__); \
  ret = 1; \
  goto end_##E; \
}

void remove_namespace(xmlNodePtr node, const xmlChar* ns) {
  if (!strcmp((const char*)node->ns->href,(const char*)ns))
    xmlSetNs(node,NULL);
  for (node = node->xmlChildrenNode; node; node = node->next)
    remove_namespace(node,ns);
}

double** parse_path(const char* d) {
  // M = move
  // L = line
  // H = horizontal line
  // V = vertical line
  // C = curve
  // S = smooth curve
  // Q = quadratic Bézier curve
  // T = smooth quadratic Bézier curve
  // A = elliptical arc
  // Z = close path

  printf("%s\n",d);

  double*  vals = malloc(1<<10);
  double** path = malloc(1<<10);

  int n = 0;
  for (char* end; d; ++d) {
    if (n) {
      double x = strtod(d,&end);
      printf("%g\n",x);

      d = end;
      --n;
    } else switch (*d) {
      case 'M':
        n = 2;
        break;
      default: ;
    }
  }

  path[0] = vals;
  return path;
}

int main(int argc, char** argv) {
  if (argc!=3) {
    printf("usage: %s file xpath\n",argv[0]);
    return 1;
  }

  int ret = 0;
  xmlInitParser();
  LIBXML_TEST_VERSION;

  // Load XML document
  xmlDocPtr doc = xmlParseFile(argv[1]);
  if (!doc)
    ERROR(1,"unable to parse file \"%s\"\n",argv[1])

  // allow xpath to match without xmlns
  { xmlNodePtr root = xmlDocGetRootElement(doc);
    const xmlChar* root_ns = root->ns->href;
    if (root_ns)
      remove_namespace(root,root_ns);
  }

  // Create xpath evaluation context
  xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
  if (!xpathCtx)
    ERROR(2,"unable to create xpath context\n")

  // Evaluate xpath expression
  const xmlChar* xpathExpr = BAD_CAST argv[2];
  xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if (!xpathObj)
    ERROR(3,"unable to evaluate xpath expression \"%s\"\n",xpathExpr)

  // get selected nodes
  xmlNodeSetPtr nodes = xpathObj->nodesetval;
  int nnodes = nodes ? nodes->nodeNr : 0;
  if (nnodes != 1)
    ERROR(4,"xpath expression \"%s\" matched %d nodes\n",xpathExpr,nnodes)

  xmlChar* d = xmlGetProp(nodes->nodeTab[0], BAD_CAST "d");
  if (d) {
    double** path = parse_path((const char*)d);

    // TODO

    free(*path);
    free(path);
    xmlFree(d);
  }

end_4:
  xmlXPathFreeObject(xpathObj);
end_3:
  xmlXPathFreeContext(xpathCtx);
end_2:
  xmlFreeDoc(doc);
end_1:
  xmlCleanupParser();
  xmlMemoryDump();
  return ret;
}

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

// for (xmlAttr* attr = root->properties; attr; attr = attr->next) {
//   printf("%s\n",attr->name);
// }

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
  xmlNodePtr root = xmlDocGetRootElement(doc);
  const xmlChar* root_ns = root->ns->href;
  if (root_ns)
    remove_namespace(root,root_ns);

  // Create xpath evaluation context
  xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
  if (!xpathCtx)
    ERROR(2,"unable to create xpath context\n")

  // if (xmlXPathRegisterNs(
  //   xpathCtx, BAD_CAST "svg", BAD_CAST "http://www.w3.org/2000/svg"
  // ))
  //   ERROR(2,"unable to register namespace svg\n")

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

  // for (int i=nnodes; i--; ) {
  // }

  const xmlChar* d = xmlGetProp(nodes->nodeTab[0], BAD_CAST "d");
  if (d)
    printf("%s\n",d);

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

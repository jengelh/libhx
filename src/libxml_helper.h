#ifndef _LIBHX_LIBXML_HELPER_H
#define _LIBHX_LIBXML_HELPER_H 1

#ifdef __cplusplus
#	include <cstring>
#else
#	include <string.h>
#endif
#include <libxml/parser.h>

static inline int xml_strcmp(const xmlChar *a, const char *b)
{
#ifdef __cplusplus
	return strcmp(reinterpret_cast<const char *>(a), b);
#else
	return strcmp((const char *)a, b);
#endif
}

static inline char *xml_getprop(xmlNode *node, const char *attr)
{
#ifdef __cplusplus
	return reinterpret_cast<char *>(xmlGetProp(node,
	       reinterpret_cast<const xmlChar *>(attr)));
#else
	return (char *)xmlGetProp(node, (const xmlChar *)attr);
#endif
}

static inline xmlAttr *xml_newprop(xmlNode *node, const char *name,
    const char *value)
{
#ifdef __cplusplus
	return xmlNewProp(node, reinterpret_cast<const xmlChar *>(name),
	       reinterpret_cast<const xmlChar *>(value));
#else
	return xmlNewProp(node, (const xmlChar *)name, (const xmlChar *)value);
#endif
}

static inline xmlNode *xml_newnode(xmlNs *ns, const char *name)
{
#ifdef __cplusplus
	return xmlNewNode(ns, reinterpret_cast<const xmlChar *>(name));
#else
	return xmlNewNode(ns, (const xmlChar *)name);
#endif
}

static inline xmlAttr *xml_setprop(xmlNode *node, const char *name,
    const char *value)
{
#ifdef __cplusplus
	return xmlSetProp(node, reinterpret_cast<const xmlChar *>(name),
	       reinterpret_cast<const xmlChar *>(value));
#else
	return xmlSetProp(node, (const xmlChar *)name, (const xmlChar *)value);
#endif
}

#endif /* _LIBHX_LIBXML_HELPER_H */

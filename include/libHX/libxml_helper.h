#ifndef _LIBHX_LIBXML_HELPER_H
#define _LIBHX_LIBXML_HELPER_H 1

#ifdef __cplusplus
#	include <cstring>
#else
#	include <string.h>
#endif
#include <libxml/parser.h>
#include <libHX/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

static __inline__ int xml_strcmp(const xmlChar *a, const char *b)
{
#ifdef __cplusplus
	return strcmp(signed_cast<const char *>(a), b);
#else
	return strcmp(signed_cast(const char *, a), b);
#endif
}

static __inline__ int xml_strcasecmp(const xmlChar *a, const char *b)
{
#ifdef __cplusplus
	return strcasecmp(signed_cast<const char *>(a), b);
#else
	return strcasecmp(signed_cast(const char *, a), b);
#endif
}

#ifdef __cplusplus
} /* extern "C" */
#endif

static __inline__ char *xml_getprop(xmlNode *node, const char *attr)
{
#ifdef __cplusplus
	return signed_cast<char *>(xmlGetProp(node,
	       signed_cast<const xmlChar *>(attr)));
#else
	return signed_cast(char *, xmlGetProp(node,
	       signed_cast(const xmlChar *, attr)));
#endif
}

/**
 * xmlGetNsProp takes, as 3rd argument, a full namespace string.
 * That is unwieldy.
 */
static __inline__ char *xml_getnsprop(xmlNode *node, const char *nsprefix,
    const char *key)
{
	const struct _xmlAttr *attr = NULL;
	for (attr = node->properties; attr != NULL; attr = attr->next)
		if (attr->ns != NULL && attr->ns->prefix != NULL &&
		    xml_strcmp(attr->ns->prefix, nsprefix) == 0)
			break;
	if (attr == NULL)
		return NULL;
#ifdef __cplusplus
	return signed_cast<char *>(xmlGetNsProp(node,
	       signed_cast<const xmlChar *>(key), attr->ns->href));
#else
	return signed_cast(char *, xmlGetNsProp(node,
	       signed_cast(const xmlChar *, key), attr->ns->href));
#endif
}

static __inline__ xmlAttr *
xml_newprop(xmlNode *node, const char *name, const char *value)
{
#ifdef __cplusplus
	return xmlNewProp(node, signed_cast<const xmlChar *>(name),
	       signed_cast<const xmlChar *>(value));
#else
	return xmlNewProp(node, signed_cast(const xmlChar *, name),
	       signed_cast(const xmlChar *, value));
#endif
}

/**
 * @ptr:	parent node
 * @name:	name of new node
 * @value:	string, or %NULL
 */
static __inline__ xmlNode *
xml_newnode(xmlNode *ptr, const char *name, const char *value)
{
#ifdef __cplusplus
	return xmlNewTextChild(ptr, NULL, signed_cast<const xmlChar *>(name),
	       signed_cast<const xmlChar *>(value));
#else
	return xmlNewTextChild(ptr, NULL, signed_cast(const xmlChar *, name),
	       signed_cast(const xmlChar *, value));
#endif
}

static __inline__ xmlAttr *
xml_setprop(xmlNode *node, const char *name, const char *value)
{
#ifdef __cplusplus
	return xmlSetProp(node, signed_cast<const xmlChar *>(name),
	       signed_cast<const xmlChar *>(value));
#else
	return xmlSetProp(node, signed_cast(const xmlChar *, name),
	       signed_cast(const xmlChar *, value));
#endif
}

#ifdef __cplusplus
static __inline__ const char *xml_getprop(const xmlNode *node, const char *attr)
{
	return xml_getprop(const_cast<xmlNode *>(node), attr);
}
static __inline__ char *xml_getnsprop(const xmlNode *node, const char *nsprefix,
    const char *attr)
{
	return xml_getnsprop(const_cast<xmlNode *>(node), nsprefix, attr);
}
#endif

#endif /* _LIBHX_LIBXML_HELPER_H */

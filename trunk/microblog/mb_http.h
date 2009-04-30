/**
*
* HTTP connection handling for Microblog
*
*/

#ifndef __MB_HTTP__
#define __MB_HTTP__

#include <glib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <glib/gi18n.h>
#include <sys/types.h>
#include <time.h>

#ifndef G_GNUC_NULL_TERMINATED
#  if __GNUC__ >= 4
#    define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#  else
#    define G_GNUC_NULL_TERMINATED
#  endif /* __GNUC__ >= 4 */
#endif /* G_GNUC_NULL_TERMINATED */

#include <sslconn.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MB_HTTPID "mb_http"

enum MbHttpStatus {
	HTTP_OK = 200,
	HTTP_MOVED_TEMPORARILY = 304,
	HTTP_NOT_FOUND = 404,
};

enum MbHttpRequestType{
	HTTP_GET = 1,
	HTTP_POST = 2,
};

enum MbHttpProto {
	MB_HTTP = 1,
	MB_HTTPS = 2,
	MB_PROTO_UNKNOWN = 100,
};

enum MbHttpState {
	MB_HTTP_STATE_INIT = 0,
	MB_HTTP_STATE_HEADER = 1,
	MB_HTTP_STATE_CONTENT = 2,
	MB_HTTP_STATE_FINISHED = 3,
};

#define MB_MAXBUFF 10240

typedef struct _MbHttpData {
	gchar * host;
	gchar * path;
	gint port;
	gint proto;
	// url = proto://host:port/path
	
	// header part
	GHashTable * headers;
	gint headers_len;
	gchar * fixed_headers;

	// param part
	GList * params;
	gint params_len;
	
	// content
	GString * content;
	// Chunked, in case of Transfer-Encoding: chunked
	GString * chunked_content;
	gint content_len;
	// For receiving side, content_len is the size of content, determined by content-length header
	// For sending side, content_len is never used.
	
	gint status;
	gint type;
	gint state;
	
	gchar * header_packet;
	gchar * packet;
	gchar * cur_packet;
	gint packet_len;
} MbHttpData;

typedef struct _MbHttpParam {
	gchar * key;
	gchar * value;
} MbHttpParam;

/*
	Create new MbHttpData
	
	@return newly created MbHttpData, use mb_http_data_free to free it afterward
*/
extern MbHttpData * mb_http_data_new(void);

/*
	Free a MbHttpData
	
	@param data MbHttpData to free
*/
extern void mb_http_data_free(MbHttpData * data);

/*
	Read a Http data from a stream
	
	@param fd file descriptor
	@param data MbHttpData
	@return number of read bytes
*/
extern gint mb_http_data_read(gint fd, MbHttpData * data);

/*
	Read a Http data from SSL stream
	
	@param ssl purple ssl stream
	@param data MbHttpData
	@return number of read bytes
*/
extern gint mb_http_data_ssl_read(PurpleSslConnection * ssl, MbHttpData * data);

/*
	Write a Http data to a stream
	
	@param fd file descriptor
	@param data MbHttpData
	@return bytes written
*/
extern gint mb_http_data_write(gint fd, MbHttpData * data);

/*
	Write a Http data to SSL stream
	
	@param ssl purple ssl stream
	@param data MbHttpData
	@return bytes written
*/
extern gint mb_http_data_ssl_write(PurpleSslConnection * ssl, MbHttpData * data);

/*
	Set URL to MbHttpData

	@param data MbHttpData
	@param url url to set (ex: https://twitter.com:80/statuses/friends_timeline.xml)
*/
extern void mb_http_data_set_url(MbHttpData * data, const gchar * url);

/*
	Get URL to MbHttpData

	@param data MbHttpData
	@param url output buffer for URL
	@param url_len length of @a url
*/
extern void mb_http_data_get_url(MbHttpData * data, gchar * url, gint url_len);

/*
	Set path for current MbHttpData
	
	@param data MbHttpData
	@param path new path to set, if old path exists, it'll be freed first.
*/
extern void mb_http_data_set_path(MbHttpData * data, const gchar * path);

/*
	Set host for current MbHttpData
	
	@param data MbHttpData
	@param path new host to set, if old path exists, it'll be freed first.
*/
extern void mb_http_data_set_host(MbHttpData * data, const gchar * host);

/*
	Set content into current content. If content already exist, it'll truncate the string first
	
	@param data MbHttpData
	@param content content to set to data->content
*/
extern void mb_http_data_set_content(MbHttpData * data, const gchar * content, gssize len);

/*
	Set HTTP Basic authen with specified user/password into header
	
	@param data MbHttpData
	@param user user name
	@param passwd password
*/
extern void mb_http_data_set_basicauth(MbHttpData * data, const gchar * user, const gchar * passwd);

/*
	Set/replace a header for HTTP connection to MbHttpData
	
	@param data MbHttpData
	@param key header
	@param value value of header
*/
extern void mb_http_data_set_header(MbHttpData* data, const gchar * key, const gchar * value);

/*
	Get current value of current header
	
	@param data MbHttpData
	@param key header to look for
	@return internal buffer pointed to header string, or NULL if not found
*/
extern gchar * mb_http_data_get_header(MbHttpData * data, const gchar * key);

/*
	Set a "fixed header" for write (outgoing) stream
	
	@param data MbHttpData
	@param headers header to set
	@note each line of header MUST ends with \r\n
*/
extern void mb_http_data_set_fixed_headers(MbHttpData * data, const gchar * headers);

/*
	Add new www-urlencoded parameter to data
	
	@param data MbHttpData
	@param key key of param
	@param value value of current param
*/
extern void mb_http_data_add_param(MbHttpData * data, const gchar * key, const gchar * value);

/*
	Add new www-urlencoded parameter to data
	
	@param data MbHttpData
	@param key key of param
	@param value value of current param
*/
extern void mb_http_data_add_param_int(MbHttpData * data, const gchar * key, gint value);

/*
	Add new www-urlencoded parameter to data
	
	@param data MbHttpData
	@param key key of param
	@param value value of current param
*/
extern void mb_http_data_add_param_ull(MbHttpData * data, const gchar * key, unsigned long long value);



/*
	Look for value of specified parameter
	
	@param data MbHttpdata
	@param key key to look for
	@return buffer of value of specified key (first occurance)
*/
extern const gchar * mb_http_data_find_param(MbHttpData * data, const gchar * key);

/*
	Remove a parameter. If multiple instance of key exists, only the first one will be removed.
	
	@param data MbHttpData
	@param key key to remove. 
	@return TRUE if key is founded and removed, FALSE if not found
*/
extern gboolean mb_http_data_rm_param(MbHttpData * data, const gchar * key);

/*
	Truncate all data and re-initialize everything back to zero
	
	@param data MbHttpData
*/
extern void mb_http_data_truncate(MbHttpData * data);

/*
   	Prepare data separately in header and packet

	The prepared data can be used for purple url fetch util
 */
extern void mb_http_data_prepare_fetch(MbHttpData * data);

#ifdef __cplusplus
}
#endif

#endif

/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 297205 2010-03-30 21:09:07Z johannes $ */

#ifndef PHP_CG_BCODE_H
#define PHP_CG_BCODE_H


#include "ext/standard/php_smart_str.h"
extern zend_module_entry cg_bcode_module_entry;
#define phpext_cg_bcode_ptr &cg_bcode_module_entry

#ifdef PHP_WIN32
#define PHP_CG_BCODE_API __declspec(dllexport)
#else
#define PHP_CG_BCODE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(cg_bcode);
PHP_MSHUTDOWN_FUNCTION(cg_bcode);
PHP_RINIT_FUNCTION(cg_bcode);
PHP_RSHUTDOWN_FUNCTION(cg_bcode);
PHP_MINFO_FUNCTION(cg_bcode);

PHP_FUNCTION(confirm_cg_bcode_compiled);	/* For testing, remove later. */

PHP_FUNCTION(bencode);
PHP_FUNCTION(bdecode);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     


ZEND_BEGIN_MODULE_GLOBALS(cg_bcode)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(cg_bcode)
*/

/* In every utility function you add that needs to use variables 
   in php_cg_bcode_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as CG_BCODE_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/


//PHPAPI char *php_bencode_string(char *str, int str_len, int * result_len);
//PHPAPI void *php_bencode_string(char *str, int *str_len);
//PHPAPI void php_bencode_string(char *str, int str_len, char **p_result, int *p_result_len);
//PHPAPI void php_bencode_string(char *str, int str_len, zval *return_value TSRMLS_DC);
//PHPAPI char *php_bencode_int(long long i);
//static void php_bencode_array(smart_str *buf, zval **val TSRMLS_DC);
static void php_bencode(smart_str *buf, zval *val TSRMLS_DC);
static void php_bdecode (       zval *return_value, char *str, int *pos TSRMLS_DC);
static void php_bdecode_int(    zval *return_value, char *str, int *pos TSRMLS_DC);
static void php_bdecode_string( zval *return_value, char *str, int *pos TSRMLS_DC);
static void php_bdecode_list(   zval *return_value, char *str, int *pos TSRMLS_DC);
static void php_bdecode_dict(   zval *return_value, char *str, int *pos TSRMLS_DC);

#ifdef ZTS
#define CG_BCODE_G(v) TSRMG(cg_bcode_globals_id, zend_cg_bcode_globals *, v)
#else
#define CG_BCODE_G(v) (cg_bcode_globals.v)
#endif

#endif	/* PHP_CG_BCODE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

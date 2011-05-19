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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_smart_str.h"
#include "php_cg_bcode.h"


/* If you declare any globals in php_cg_bcode.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(cg_bcode)
*/

/* True global resources - no need for thread safety here */
static int le_cg_bcode;

/* {{{ cg_bcode_functions[]
 *
 * Every user visible function must have an entry in cg_bcode_functions[].
 */
zend_function_entry cg_bcode_functions[] = {
    PHP_FE(bencode,    NULL)        /* For testing, remove later. */
    PHP_FE(bdecode,    NULL)        /* For testing, remove later. */
    {NULL, NULL, NULL}    /* Must be the last line in cg_bcode_functions[] */
};
/* }}} */

/* {{{ cg_bcode_module_entry
 */
zend_module_entry cg_bcode_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "cg_bcode",
    cg_bcode_functions,
    PHP_MINIT(cg_bcode),
    PHP_MSHUTDOWN(cg_bcode),
    PHP_RINIT(cg_bcode),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(cg_bcode),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(cg_bcode),
#if ZEND_MODULE_API_NO >= 20010901
    "0.1", /* Replace with version number for your extension */
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CG_BCODE
ZEND_GET_MODULE(cg_bcode)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("cg_bcode.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_cg_bcode_globals, cg_bcode_globals)
    STD_PHP_INI_ENTRY("cg_bcode.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_cg_bcode_globals, cg_bcode_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_cg_bcode_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_cg_bcode_init_globals(zend_cg_bcode_globals *cg_bcode_globals)
{
    cg_bcode_globals->global_value = 0;
    cg_bcode_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(cg_bcode)
{
    /* If you have INI entries, uncomment these lines 
    REGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(cg_bcode)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(cg_bcode)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(cg_bcode)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(cg_bcode)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "cg_bcode support", "enabled");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

static int json_determine_array_type(zval **val TSRMLS_DC)  /* {{{ */
{
    int i;
    HashTable *myht = HASH_OF(*val);

    i = myht ? zend_hash_num_elements(myht) : 0;
    if (i > 0) {
        char *key;
        ulong index, idx;
        uint key_len;
        HashPosition pos;

        zend_hash_internal_pointer_reset_ex(myht, &pos);
        idx = 0;
        for (;; zend_hash_move_forward_ex(myht, &pos)) {
            i = zend_hash_get_current_key_ex(myht, &key, &key_len, &index, 0, &pos);
            if (i == HASH_KEY_NON_EXISTANT)
                break;

            if (i == HASH_KEY_IS_STRING) {
                return 1;
            } else {
                if (index != idx) {
                    return 1;
                }
            }
            idx++;
        }
    }

    return 0;
}
/* }}} */


static void php_bencode_array(smart_str *buf, zval **val TSRMLS_DC)  /* {{{ */
{
    int i, r;
    HashTable *myht;

    if (Z_TYPE_PP(val) == IS_ARRAY) {
        myht = HASH_OF(*val);
        r = json_determine_array_type(val TSRMLS_CC);
    } else {
        myht = Z_OBJPROP_PP(val);
        r = 1;
    }

    if (myht && myht->nApplyCount > 1) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "recursion detected");
        smart_str_appends(buf, "");
        return;
    }

    if (r == 0)
    {
        smart_str_appendc(buf, 'l');
    }
    else
    {
        smart_str_appendc(buf, 'd');
    }

    i = myht ? zend_hash_num_elements(myht) : 0;
    if (i > 0) {
        char *key;
        zval **data;
        long index;
        uint key_len;
        HashPosition pos;
        HashTable *tmp_ht;

        zend_hash_internal_pointer_reset_ex(myht, &pos);
        for (;; zend_hash_move_forward_ex(myht, &pos)) {
            i = zend_hash_get_current_key_ex(myht, &key, &key_len, &index, 0, &pos);
            if (i == HASH_KEY_NON_EXISTANT)
                break;

            if (zend_hash_get_current_data_ex(myht, (void **) &data, &pos) == SUCCESS) {
                tmp_ht = HASH_OF(*data);
                if (tmp_ht) {
                    tmp_ht->nApplyCount++;
                }

                if (r == 0) {
                    php_bencode(buf, *data TSRMLS_CC);
                } else if (r == 1) {
                    if (i == HASH_KEY_IS_STRING) {
                        if (key[0] == '\0' && Z_TYPE_PP(val) == IS_OBJECT) {
                            /* Skip protected and private members. */
							if (tmp_ht) {
								tmp_ht->nApplyCount--;
							}
                            continue;
                        }
                                                
                        smart_str_append_long(buf, key_len -1);
                        smart_str_appendc(buf, ':');
                        smart_str_appendl(buf, key, key_len - 1);
                        
                        php_bencode(buf, *data TSRMLS_CC);

                    } else {

                        char *tmp_len;
			long newlen;
			newlen = spprintf(&tmp_len, 0, "%ld", index);
                        smart_str_append_long(buf, newlen);
                        efree(tmp_len);
                        smart_str_appendc(buf, ':');
                        smart_str_append_long(buf, (long) index);

                        php_bencode(buf, *data TSRMLS_CC);
                    }
                }

                if (tmp_ht) {
                    tmp_ht->nApplyCount--;
                }
            }
        }
    }

    smart_str_appendc(buf, 'e');
}

PHP_FUNCTION(bencode) 
{
    zval *parameter;
    smart_str buf = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &parameter) == FAILURE) {
        return;
    }

    php_bencode(&buf, parameter TSRMLS_CC);

    ZVAL_STRINGL(return_value, buf.c, buf.len, 1);

    smart_str_free(&buf);
    //zval_ptr_dtor(&parameter);
}

static void php_bencode(smart_str *buf, zval *val TSRMLS_DC) /* {{{ */
{
    switch (Z_TYPE_P(val)) {
        case IS_NULL:
            smart_str_appendl(buf, "", 0);
            break;
        case IS_BOOL:
            if (Z_BVAL_P(val))
            {
                smart_str_appendl(buf, "i1e", 3);
            }
            else
            {
                smart_str_appendl(buf, "i0e", 3);
            }
            break;
        case IS_LONG:
            smart_str_appendc(buf, 'i');
            smart_str_append_long(buf, Z_LVAL_P(val));
            smart_str_appendc(buf, 'e');
            break;
        case IS_DOUBLE:
            {
		char *d = NULL;
                int len;
                double dbl = Z_DVAL_P(val);

                if (!zend_isinf(dbl) && !zend_isnan(dbl)) {
                        len = spprintf(&d, 0, "%lli", (long long)dbl);
			smart_str_appendc(buf, 'i');
                        smart_str_appendl(buf, d, len);
			smart_str_appendc(buf, 'e');
                        efree(d);
                } else {
                    zend_error(E_WARNING, "[cg_bcode] (php_bencode) double %.9g does not conform to the bencode spec, encoded as 0", dbl);
                    smart_str_appendc(buf, '0');
                }
        
            }
            break;
        case IS_STRING:
            smart_str_append_long(buf, Z_STRLEN_P(val));
            smart_str_appendc(buf, ':');
            smart_str_appendl(buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
            break;
        case IS_ARRAY:
        case IS_OBJECT:
            php_bencode_array(buf, &val TSRMLS_CC);
            break;
        default:
            zend_error(E_WARNING, "[cg_bcode] (php_bencode) type is unsupported, encoded as null");
            smart_str_appendl(buf, "", 0);
            break;
    }
    return;
}




PHP_FUNCTION(bdecode)
{
    char *parameter;
    int parameter_len;
    int pos = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &parameter, &parameter_len) == FAILURE) {
        return;
    }

    if (!parameter_len)
    {
        RETURN_NULL();
    }
    php_bdecode(return_value, parameter, &pos TSRMLS_CC);
}

static void php_bdecode(zval *return_value, char *str, int *pos TSRMLS_DC)
{
	switch (str[*pos])
	{
		case 'd':
		{
			php_bdecode_dict(return_value, str, pos TSRMLS_CC);
		}
		break;
		case 'l':
		{
			php_bdecode_list(return_value, str, pos TSRMLS_CC);
		}
		break;
		case 'i':
		{
			php_bdecode_int(return_value, str, pos TSRMLS_CC);
		}
		break;
		default :
		{
			php_bdecode_string(return_value, str, pos TSRMLS_CC);
		}
		break;
	}	
	//zval_copy_ctor(return_value);
    //zval_ptr_dtor(&ret);
}


static void php_bdecode_int(zval *return_value, char *str, int *pos TSRMLS_DC)
{
	smart_str buf = {0};
	(*pos)++;
	while (str[*pos] != 'e')
	{
		smart_str_appendc(&buf, str[*pos]);
		(*pos)++;
	}
	(*pos)++;
	smart_str_0(&buf);

	double d = zend_strtod(buf.c, NULL);
	if (d > LONG_MAX || d < -LONG_MAX) 
	{
		ZVAL_STRINGL(return_value, buf.c, buf.len, 1);
		convert_to_double(return_value);
	}
	else
	{
		ZVAL_STRINGL(return_value, buf.c, buf.len, 1);
		convert_to_long(return_value);
	}
	smart_str_free(&buf);
}

static void php_bdecode_string(zval *return_value, char *str, int *pos TSRMLS_DC)
{
	int i = 0;
	int len = 0;

	smart_str buf = {0};
	while(str[*pos] != ':')
	{
		smart_str_appendc(&buf, str[*pos]);					
		(*pos)++;
	}
	smart_str_0(&buf);
	len = atoi(buf.c);
	smart_str_free(&buf);
	(*pos)++;
	for (i = 0; i < len; i++)
	{
		smart_str_appendc(&buf, str[*pos]);
        (*pos)++;
	}
	smart_str_0(&buf);
    ZVAL_STRINGL(return_value, buf.c, buf.len, 1);
    smart_str_free(&buf);
}

static void php_bdecode_list(zval *return_value, char *str, int *pos TSRMLS_DC)
{
    zval *list;
	ALLOC_INIT_ZVAL(list);
	array_init(list);

	(*pos)++;
	while (str[*pos] != 'e')
	{
		zval *v;
		ALLOC_INIT_ZVAL(v);
		php_bdecode(v, str, pos TSRMLS_CC);
	
		switch (Z_TYPE_P(v)) 
		{					
			case IS_STRING:
				add_next_index_stringl(list, Z_STRVAL_P(v), Z_STRLEN_P(v), 1);
       			zval_ptr_dtor(&v);
				break;
			case IS_LONG:
				add_next_index_long(list, Z_LVAL_P(v));
        		zval_ptr_dtor(&v);
				break;
			case IS_DOUBLE:
				add_next_index_double(list, Z_DVAL_P(v));
        		zval_ptr_dtor(&v);
				break;
          	case IS_ARRAY:
				add_next_index_zval(list,  v);
				break;
			default:
				break;
		}
	}
	(*pos)++;
	*return_value = *list;
    zval_copy_ctor(return_value);
    zval_ptr_dtor(&list);
}

static void php_bdecode_dict(zval *return_value, char *str, int *pos TSRMLS_DC)
{
    zval *dict;
	ALLOC_INIT_ZVAL(dict);
	array_init(dict);
 
	(*pos)++;
   
	while (str[*pos] != 'e')
	{
	    zval *v;
    	zval *k;
		ALLOC_INIT_ZVAL(v);
		ALLOC_INIT_ZVAL(k);

		php_bdecode_string(k, str, pos TSRMLS_CC);
		php_bdecode(v, str, pos TSRMLS_CC);

		switch (Z_TYPE_P(v)) 
		{					
			case IS_STRING:                               
				add_assoc_stringl(dict, Z_STRVAL_P(k), Z_STRVAL_P(v), Z_STRLEN_P(v), 1);
				zval_ptr_dtor(&v);
				break;
			case IS_DOUBLE:
				add_assoc_double(dict,Z_STRVAL_P(k), Z_DVAL_P(v));
        		zval_ptr_dtor(&v);
				break;			
			case IS_LONG:
				add_assoc_long(dict, Z_STRVAL_P(k), Z_LVAL_P(v));
				zval_ptr_dtor(&v);
      			break;
			case IS_ARRAY:
				add_assoc_zval(dict, Z_STRVAL_P(k), v);
				break;
		}
		zval_ptr_dtor(&k);
	}
	*return_value = *dict;
    zval_copy_ctor(return_value);
    zval_ptr_dtor(&dict);
	(*pos)++;
}





/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */



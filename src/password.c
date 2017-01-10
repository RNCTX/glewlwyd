/**
 *
 * Glewlwyd OAuth2 Authorization Server
 *
 * OAuth2 authentiation server
 * Users are authenticated with a LDAP server
 * or users stored in the database 
 * Provides Json Web Tokens (jwt)
 * 
 * password manager functions, most of this code is inspired by Barry Steyn,
 * under the MIT licence, thanks to him
 *
 * Copyright 2016 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU GENERAL PUBLIC LICENSE
 * License as published by the Free Software Foundation;
 * version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <openssl/ssl.h>

#include "glewlwyd.h"

/**
 * Generates a random string and store it in str
 */
char * rand_salt(char * str, size_t str_size) {
    size_t n;
    
    if (str_size > 0 && str != NULL) {
        for (n = 0; n < str_size; n++) {
            int key = rand()+1;
            str[n] = key;
        }
        str[str_size] = '\0';
        return str;
    } else {
      return NULL;
    }
}

int generate_digest(digest_algorithm digest, const char * password, int use_salt, char * out_password) {
  EVP_MD_CTX mdctx;
  const EVP_MD *md;
  unsigned char md_value[1024];
  unsigned int md_len, res;
  
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;

  char * intermediate, buffer[1024 + GLEWLWYD_SALT_LENGTH + 1] = {0}, salt[GLEWLWYD_SALT_LENGTH + 1] = {0};
  if (password == NULL || out_password == NULL) {
    res = 0;
  } else {
    intermediate = malloc(strlen(password)+((GLEWLWYD_SALT_LENGTH+1)*sizeof(char)));
    if (intermediate != NULL) {
      switch (digest) {
        case digest_SHA1:
          md = EVP_sha1();
          break;
        case digest_SHA224:
          md = EVP_sha224();
          break;
        case digest_SHA256:
          md = EVP_sha256();
          break;
        case digest_SHA384:
          md = EVP_sha384();
          break;
        case digest_SHA512:
          md = EVP_sha512();
          break;
        case digest_MD5:
          md = EVP_md5();
          break;
        default:
          md = NULL;
          break;
      }
      //md = EVP_get_digestbyname(digest);
      if(md == NULL) {
        res = 0;
      }
      sprintf(intermediate, "%s", password);
      
      if (use_salt) {
        rand_salt(salt, GLEWLWYD_SALT_LENGTH);
        strncat(intermediate, salt, GLEWLWYD_SALT_LENGTH);
      }
      
      EVP_MD_CTX_init(&mdctx);
      EVP_DigestInit_ex(&mdctx, md, NULL);
      EVP_DigestUpdate(&mdctx,
                       intermediate,
                       (unsigned int) strlen(intermediate));
      EVP_DigestFinal_ex(&mdctx,
                         md_value,
                         &md_len);
      
      strncpy(buffer, (char *)md_value, md_len);
      if (use_salt) {
        strncat(buffer, salt, GLEWLWYD_SALT_LENGTH);
      }

      b64 = BIO_new(BIO_f_base64());
      bio = BIO_new(BIO_s_mem());
      bio = BIO_push(b64, bio);

      BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
      BIO_write(bio, buffer, strlen(buffer));
      BIO_flush(bio);
      BIO_get_mem_ptr(bio, &bufferPtr);

      strncpy(out_password, (*bufferPtr).data, (*bufferPtr).length);
      
      BIO_set_close(bio, BIO_CLOSE);
      BIO_free_all(bio);
      EVP_MD_CTX_cleanup(&mdctx);
      res = 1;
    } else {
      res = 0;
    }
    free(intermediate);
  }
  return res;
}

/**
 * Generates a random string and store it in str
 */
char * rand_string(char * str, size_t str_size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.\"!/$%?&*()_-+=<>{}[]'";
    size_t n;
    
    if (str_size > 0 && str != NULL) {
        for (n = 0; n < str_size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[str_size] = '\0';
        return str;
    } else {
      return NULL;
    }
}

/**
 * Generates a random string and store it in str
 */
char * rand_crypt_salt(char * str, size_t str_size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    size_t n;
    
    if (str_size > 0 && str != NULL) {
        for (n = 0; n < str_size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[str_size] = '\0';
        return str;
    } else {
      return NULL;
    }
}

char * generate_hash(struct config_elements * config, const char * digest, const char * password) {
  char * to_return = NULL, buffer[1024] = {0};
  char salt[GLEWLWYD_SALT_LENGTH + 1] = {0};
  
  if (digest != NULL && password != NULL) {
    if (!strcmp(digest, "SSHA")) {
      generate_digest(digest_SHA1, password, 1, buffer);
      to_return = msprintf("{SSHA}%s", buffer, strlen(buffer));
    } else if (!strcmp(digest, "SHA1")) {
      generate_digest(digest_SHA1, password, 0, buffer);
      to_return = msprintf("{SHA}%s", buffer);
    } else if (!strcmp(digest, "SHA224")) {
      generate_digest(digest_SHA224, password, 0, buffer);
      to_return = msprintf("{SHA224}%s", buffer);
    } else if (!strcmp(digest, "SSHA224")) {
      generate_digest(digest_SHA224, password, 1, buffer);
      to_return = msprintf("{SSHA224}%s", buffer);
    } else if (!strcmp(digest, "SHA256")) {
      generate_digest(digest_SHA256, password, 0, buffer);
      to_return = msprintf("{SHA256}%s", buffer);
    } else if (!strcmp(digest, "SSHA256")) {
      generate_digest(digest_SHA256, password, 1, buffer);
      to_return = msprintf("{SSHA256}%s", buffer);
    } else if (!strcmp(digest, "SHA384")) {
      generate_digest(digest_SHA384, password, 0, buffer);
      to_return = msprintf("{SHA384}%s", buffer);
    } else if (!strcmp(digest, "SSHA384")) {
      generate_digest(digest_SHA384, password, 1, buffer);
      to_return = msprintf("{SSHA384}%s", buffer);
    } else if (!strcmp(digest, "SHA512")) {
      generate_digest(digest_SHA512, password, 0, buffer);
      to_return = msprintf("{SHA512}%s", buffer);
    } else if (!strcmp(digest, "SSHA512")) {
      generate_digest(digest_SHA512, password, 1, buffer);
      to_return = msprintf("{SSHA512}%s", buffer);
    } else if (!strcmp(digest, "SMD5")) {
      generate_digest(digest_MD5, password, 1, buffer);
      to_return = msprintf("{SMD5}%s", buffer);
    } else if (!strcmp(digest, "MD5")) {
      generate_digest(digest_MD5, password, 0, buffer);
      to_return = msprintf("{MD5}%s", buffer);
    } else if (!strcmp(digest, "CRYPT")) {
      rand_crypt_salt(salt, GLEWLWYD_SALT_LENGTH);
      to_return = msprintf("{CRYPT}%s", crypt_r(password, salt, &config->auth_ldap->cur_crypt_data));
    } else {
      to_return = strdup(password);
    }
  }
  return to_return;
}
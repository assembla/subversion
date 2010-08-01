/**
 * @copyright
 * ====================================================================
 *    Licensed to the Apache Software Foundation (ASF) under one
 *    or more contributor license agreements.  See the NOTICE file
 *    distributed with this work for additional information
 *    regarding copyright ownership.  The ASF licenses this file
 *    to you under the Apache License, Version 2.0 (the
 *    "License"); you may not use this file except in compliance
 *    with the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing,
 *    software distributed under the License is distributed on an
 *    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *    KIND, either express or implied.  See the License for the
 *    specific language governing permissions and limitations
 *    under the License.
 * ====================================================================
 * @endcopyright
 *
 * @file svn_serializer.h
 * @brief Helper API for serializing _temporarily_ data structures.
 *
 * @note This API is intended for efficient serialization and duplication
 *       of temporary, e.g. cached, data structures ONLY. It is not
 *       suitable for persistent data.
 */

#include <apr.h>
#include "svn_types.h"

/* forward declaration */
struct svn_stringbuf_t;

/**
 * Opaque structure controlling the serialization process and holding the
 * intermediate as well as final results.
 */
typedef struct svn_temp_serializer__context_t svn_temp_serializer__context_t;

/**
 * Begin the serialization process for the @a source_struct and all objects
 * referenced from it. @a struct_size must match the result of @c sizeof()
 * of the actual structure. Due to the generic nature of the init function
 * we can't determine the structure size as part of the function.
 *
 * You may suggest a larger initial buffer size in @a suggested_buffer_size
 * to minimize the number of internal buffer re-allocations during the
 * serialization process. All allocations will be made from @a pool.
 *
 * Pointers within the structure will be replaced by their serialized
 * representation when the respective strings or sub-structures get
 * serialized. This scheme allows only for tree-like, i.e. non-circular
 * data structures.
 *
 * @return the serization context.
 */
svn_temp_serializer__context_t *
svn_temp_serializer__init(const void *source_struct,
                          apr_size_t struct_size,
                          apr_size_t suggested_buffer_size,
                          apr_pool_t *pool);

/**
 * Begin serialization of a referenced sub-structure within the
 * serialization @a context. @a source_struct must be a reference to the
 * pointer in the original parent structure so that the correspondence in
 * the serialized structure can be established. @a struct_size must match
 * the result of @c sizeof() of the actual structure.
 *
 * Sub-structures and strings will be added in a FIFO fashion. If you need
 * add further sub-structures on the same level, you need to call @ref
 * svn_serializer__pop to realign the serialization context.
 */
void
svn_temp_serializer__push(svn_temp_serializer__context_t *context,
                          const void * const * source_struct,
                          apr_size_t struct_size);

/**
 * End the serialization of the current sub-structure. The serialization
 * @c context will be focussed back on the parent structure. You may then
 * add further sub-structures starting from that level.
 *
 * It is not necessary to call this function just for symmetry at the end
 * of the serialization process.
 */
void
svn_temp_serializer__pop(svn_temp_serializer__context_t *context);

/**
 * Serialize a string referenced from the current structure within the
 * serialization @a context. @a s must be a reference to the @c char*
 * pointer in the original structure so that the correspondence in the
 * serialized structure can be established.
 */
void
svn_temp_serializer__add_string(svn_temp_serializer__context_t *context,
                                const char * const * s);

/**
 * @return a reference to the data buffer containing the data serialialized
 * so far in the given serialization @a context.
 */
struct svn_stringbuf_t *
svn_temp_serializer__get(svn_temp_serializer__context_t *context);

/**
 * Deserialization is straightforward: just copy the serialized buffer to
 * a natively aligned memory location (APR pools will take care of that
 * automatically) and resolve all pointers to sub-structures.
 *
 * To do the latter, call this function for each of these pointers, giving
 * the start address of the copyied buffer in @a buffer and a reference to
 * the pointer to resolve in @a ptr.
 */
void
svn_temp_deserializer__resolve(void *buffer, void **ptr);

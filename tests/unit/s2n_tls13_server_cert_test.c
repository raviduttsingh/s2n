/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "s2n_test.h"

#include <string.h>
#include <stdio.h>
#include <s2n.h>

#include "stuffer/s2n_stuffer.h"
#include "testlib/s2n_testlib.h"
#include "tls/s2n_tls.h"
#include "utils/s2n_safety.h"
#include "crypto/s2n_certificate.h"

/* Test vectors from https://tools.ietf.org/html/rfc8448#section-3 */
const char tls13_cert_hex[] =
    "3082" /* without chertificate chain header */
    "01ac30820115a003020102020102300d06092a8648"
    "86f70d01010b0500300e310c300a06035504031303"
    "727361301e170d3136303733303031323335395a17"
    "0d3236303733303031323335395a300e310c300a06"
    "03550403130372736130819f300d06092a864886f7"
    "0d010101050003818d0030818902818100b4bb498f"
    "8279303d980836399b36c6988c0c68de55e1bdb826"
    "d3901a2461eafd2de49a91d015abbc9a95137ace6c"
    "1af19eaa6af98c7ced43120998e187a80ee0ccb052"
    "4b1b018c3e0b63264d449a6d38e22a5fda43084674"
    "8030530ef0461c8ca9d9efbfae8ea6d1d03e2bd193"
    "eff0ab9a8002c47428a6d35a8d88d79f7f1e3f0203"
    "010001a31a301830090603551d1304023000300b06"
    "03551d0f0404030205a0300d06092a864886f70d01"
    "010b05000381810085aad2a0e5b9276b908c65f73a"
    "7267170618a54c5f8a7b337d2df7a594365417f2ea"
    "e8f8a58c8f8172f9319cf36b7fd6c55b80f21a0301"
    "5156726096fd335e5e67f2dbf102702e608ccae6be"
    "c1fc63a42a99be5c3eb7107c3c54e9b9eb2bd5203b"
    "1c3b84e0a8b2f759409ba3eac9d91d402dcc0cc8f8"
    "961229ac9187b42b4de10000";

/* certificate chain header. It contains
   1. Request Context lenght (00)
   2. Cert chain length (00001b5)
   3. Cert lenght (0001b0)
 */
const char tls13_cert_chain_header_hex[] =
     "000001b50001b0";

int main(int argc, char **argv)
{
    char *tls13_cert_chain_hex;
    BEGIN_TEST();
    EXPECT_NOT_NULL(tls13_cert_chain_hex = malloc(S2N_MAX_TEST_PEM_SIZE));
    strcpy(tls13_cert_chain_hex, tls13_cert_chain_header_hex);
    strcat(tls13_cert_chain_hex, tls13_cert_hex);
    struct s2n_blob tls13_cert = {0};
    EXPECT_SUCCESS(s2n_blob_zero(&tls13_cert));
    EXPECT_SUCCESS(s2n_alloc(&tls13_cert, (strlen(tls13_cert_chain_hex) / 2) ));
    GUARD(s2n_hex_string_to_bytes(tls13_cert_chain_hex, &tls13_cert));

    /* Test s2n_server_cert_recv() parses tls13 certificate */
    {
        struct s2n_connection *conn;
        EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_CLIENT));

        conn->x509_validator.skip_cert_validation = 1;

        /* success case in tls13 parsing mode */
        conn->actual_protocol_version = S2N_TLS13;
        EXPECT_EQUAL(conn->actual_protocol_version, S2N_TLS13);
        EXPECT_SUCCESS(s2n_stuffer_write(&conn->handshake.io, &tls13_cert));
        EXPECT_SUCCESS(s2n_server_cert_recv(conn));
        EXPECT_EQUAL(s2n_stuffer_data_available(&conn->handshake.io), 0);

        /* failure case in tls12 parsing mode */
        conn->actual_protocol_version = S2N_TLS12;
        EXPECT_EQUAL(conn->actual_protocol_version, S2N_TLS12);
        EXPECT_SUCCESS(s2n_stuffer_write(&conn->handshake.io, &tls13_cert));
        EXPECT_FAILURE(s2n_server_cert_recv(conn));

        EXPECT_SUCCESS(s2n_connection_free(conn));
    }

    /* Test s2n_server_cert_send() verify server's certificate */
    {
        S2N_BLOB_FROM_HEX(tls13_cert_chain, tls13_cert_hex);

        struct s2n_connection *conn;
<<<<<<< HEAD
        EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_SERVER));
        conn->actual_protocol_version = S2N_TLS13;
=======
        uint8_t certificate_request_context_len;
>>>>>>> adding support for server certificate request context (#1201)

        struct s2n_cert cert = {.raw = tls13_cert_chain, .next= NULL};
        struct s2n_cert_chain cert_chain = {.head = &cert};
        struct s2n_cert_chain_and_key cert_chain_and_key = {.cert_chain = &cert_chain};

        /* tls13 mode */
        EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_SERVER));
        conn->actual_protocol_version = S2N_TLS13;
        conn->handshake_params.our_chain_and_key = &cert_chain_and_key;
        EXPECT_EQUAL(conn->actual_protocol_version, S2N_TLS13);
        EXPECT_SUCCESS(s2n_server_cert_send(conn));
        EXPECT_EQUAL(s2n_stuffer_data_available(&conn->handshake.io), tls13_cert.size);
        GUARD(s2n_stuffer_read_uint8(&conn->handshake.io, &certificate_request_context_len));
        /* server's certificate request context should always be of zero length */
        EXPECT_EQUAL(certificate_request_context_len, 0);
        EXPECT_SUCCESS(s2n_connection_free(conn));

        /* tls12 mode */
        EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_SERVER));
        conn->actual_protocol_version = S2N_TLS12;
        conn->handshake_params.our_chain_and_key = &cert_chain_and_key;
        EXPECT_EQUAL(conn->actual_protocol_version, S2N_TLS12);
        EXPECT_SUCCESS(s2n_server_cert_send(conn));
        EXPECT_EQUAL(s2n_stuffer_data_available(&conn->handshake.io), tls13_cert.size - 1);
        EXPECT_SUCCESS(s2n_connection_free(conn));
    }

    free(tls13_cert_chain_hex);
    /* free momery allocated in s2n_alloc*/
    free(tls13_cert.data);
    END_TEST();
}

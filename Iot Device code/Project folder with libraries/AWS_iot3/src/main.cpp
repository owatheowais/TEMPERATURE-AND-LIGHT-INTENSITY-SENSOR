/* ESP8266 AWS IoT example by Evandro Luis Copercini
   Public Domain - 2017
    It connects to AWS IoT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
  -WARNING: this example doesn't verify the server CA due low heap, this can be a security issue
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h> //Get it from here: https://github.com/knolleary/pubsubclient

/* convert and replace with your keys
  $ openssl x509 -in aaaaaaaaa-certificate.pem.crt -out cert.der -outform DER
  $ openssl rsa -in aaaaaaaaaa-private.pem.key -out private.der -outform DER
  $ xxd -i certificates
*/
unsigned char certificates_esp8266_bin_CA[] =
{ 0x30, 0x82, 0x04, 0xd3, 0x30, 0x82, 0x03, 0xbb, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x10, 0x18, 0xda, 0xd1, 0x9e, 0x26, 0x7d, 0xe8, 0xbb, 0x4a,
  0x21, 0x58, 0xcd, 0xcc, 0x6b, 0x3b, 0x4a, 0x30, 0x0d, 0x06, 0x09, 0x2a,
  0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x30, 0x81,
  0xca, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02,
  0x55, 0x53, 0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13,
  0x0e, 0x56, 0x65, 0x72, 0x69, 0x53, 0x69, 0x67, 0x6e, 0x2c, 0x20, 0x49,
  0x6e, 0x63, 0x2e, 0x31, 0x1f, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x04, 0x0b,
  0x13, 0x16, 0x56, 0x65, 0x72, 0x69, 0x53, 0x69, 0x67, 0x6e, 0x20, 0x54,
  0x72, 0x75, 0x73, 0x74, 0x20, 0x4e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b,
  0x31, 0x3a, 0x30, 0x38, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x31, 0x28,
  0x63, 0x29, 0x20, 0x32, 0x30, 0x30, 0x36, 0x20, 0x56, 0x65, 0x72, 0x69,
  0x53, 0x69, 0x67, 0x6e, 0x2c, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x20, 0x2d,
  0x20, 0x46, 0x6f, 0x72, 0x20, 0x61, 0x75, 0x74, 0x68, 0x6f, 0x72, 0x69,
  0x7a, 0x65, 0x64, 0x20, 0x75, 0x73, 0x65, 0x20, 0x6f, 0x6e, 0x6c, 0x79,
  0x31, 0x45, 0x30, 0x43, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x3c, 0x56,
  0x65, 0x72, 0x69, 0x53, 0x69, 0x67, 0x6e, 0x20, 0x43, 0x6c, 0x61, 0x73,
  0x73, 0x20, 0x33, 0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20, 0x50,
  0x72, 0x69, 0x6d, 0x61, 0x72, 0x79, 0x20, 0x43, 0x65, 0x72, 0x74, 0x69,
  0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x41, 0x75, 0x74,
  0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x20, 0x2d, 0x20, 0x47, 0x35, 0x30,
  0x1e, 0x17, 0x0d, 0x30, 0x36, 0x31, 0x31, 0x30, 0x38, 0x30, 0x30, 0x30,
  0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x33, 0x36, 0x30, 0x37, 0x31, 0x36,
  0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x81, 0xca, 0x31, 0x0b,
  0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31,
  0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0e, 0x56, 0x65,
  0x72, 0x69, 0x53, 0x69, 0x67, 0x6e, 0x2c, 0x20, 0x49, 0x6e, 0x63, 0x2e,
  0x31, 0x1f, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x16, 0x56,
  0x65, 0x72, 0x69, 0x53, 0x69, 0x67, 0x6e, 0x20, 0x54, 0x72, 0x75, 0x73,
  0x74, 0x20, 0x4e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x31, 0x3a, 0x30,
  0x38, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x31, 0x28, 0x63, 0x29, 0x20,
  0x32, 0x30, 0x30, 0x36, 0x20, 0x56, 0x65, 0x72, 0x69, 0x53, 0x69, 0x67,
  0x6e, 0x2c, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x20, 0x2d, 0x20, 0x46, 0x6f,
  0x72, 0x20, 0x61, 0x75, 0x74, 0x68, 0x6f, 0x72, 0x69, 0x7a, 0x65, 0x64,
  0x20, 0x75, 0x73, 0x65, 0x20, 0x6f, 0x6e, 0x6c, 0x79, 0x31, 0x45, 0x30,
  0x43, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x3c, 0x56, 0x65, 0x72, 0x69,
  0x53, 0x69, 0x67, 0x6e, 0x20, 0x43, 0x6c, 0x61, 0x73, 0x73, 0x20, 0x33,
  0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20, 0x50, 0x72, 0x69, 0x6d,
  0x61, 0x72, 0x79, 0x20, 0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63,
  0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x41, 0x75, 0x74, 0x68, 0x6f, 0x72,
  0x69, 0x74, 0x79, 0x20, 0x2d, 0x20, 0x47, 0x35, 0x30, 0x82, 0x01, 0x22,
  0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
  0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a,
  0x02, 0x82, 0x01, 0x01, 0x00, 0xaf, 0x24, 0x08, 0x08, 0x29, 0x7a, 0x35,
  0x9e, 0x60, 0x0c, 0xaa, 0xe7, 0x4b, 0x3b, 0x4e, 0xdc, 0x7c, 0xbc, 0x3c,
  0x45, 0x1c, 0xbb, 0x2b, 0xe0, 0xfe, 0x29, 0x02, 0xf9, 0x57, 0x08, 0xa3,
  0x64, 0x85, 0x15, 0x27, 0xf5, 0xf1, 0xad, 0xc8, 0x31, 0x89, 0x5d, 0x22,
  0xe8, 0x2a, 0xaa, 0xa6, 0x42, 0xb3, 0x8f, 0xf8, 0xb9, 0x55, 0xb7, 0xb1,
  0xb7, 0x4b, 0xb3, 0xfe, 0x8f, 0x7e, 0x07, 0x57, 0xec, 0xef, 0x43, 0xdb,
  0x66, 0x62, 0x15, 0x61, 0xcf, 0x60, 0x0d, 0xa4, 0xd8, 0xde, 0xf8, 0xe0,
  0xc3, 0x62, 0x08, 0x3d, 0x54, 0x13, 0xeb, 0x49, 0xca, 0x59, 0x54, 0x85,
  0x26, 0xe5, 0x2b, 0x8f, 0x1b, 0x9f, 0xeb, 0xf5, 0xa1, 0x91, 0xc2, 0x33,
  0x49, 0xd8, 0x43, 0x63, 0x6a, 0x52, 0x4b, 0xd2, 0x8f, 0xe8, 0x70, 0x51,
  0x4d, 0xd1, 0x89, 0x69, 0x7b, 0xc7, 0x70, 0xf6, 0xb3, 0xdc, 0x12, 0x74,
  0xdb, 0x7b, 0x5d, 0x4b, 0x56, 0xd3, 0x96, 0xbf, 0x15, 0x77, 0xa1, 0xb0,
  0xf4, 0xa2, 0x25, 0xf2, 0xaf, 0x1c, 0x92, 0x67, 0x18, 0xe5, 0xf4, 0x06,
  0x04, 0xef, 0x90, 0xb9, 0xe4, 0x00, 0xe4, 0xdd, 0x3a, 0xb5, 0x19, 0xff,
  0x02, 0xba, 0xf4, 0x3c, 0xee, 0xe0, 0x8b, 0xeb, 0x37, 0x8b, 0xec, 0xf4,
  0xd7, 0xac, 0xf2, 0xf6, 0xf0, 0x3d, 0xaf, 0xdd, 0x75, 0x91, 0x33, 0x19,
  0x1d, 0x1c, 0x40, 0xcb, 0x74, 0x24, 0x19, 0x21, 0x93, 0xd9, 0x14, 0xfe,
  0xac, 0x2a, 0x52, 0xc7, 0x8f, 0xd5, 0x04, 0x49, 0xe4, 0x8d, 0x63, 0x47,
  0x88, 0x3c, 0x69, 0x83, 0xcb, 0xfe, 0x47, 0xbd, 0x2b, 0x7e, 0x4f, 0xc5,
  0x95, 0xae, 0x0e, 0x9d, 0xd4, 0xd1, 0x43, 0xc0, 0x67, 0x73, 0xe3, 0x14,
  0x08, 0x7e, 0xe5, 0x3f, 0x9f, 0x73, 0xb8, 0x33, 0x0a, 0xcf, 0x5d, 0x3f,
  0x34, 0x87, 0x96, 0x8a, 0xee, 0x53, 0xe8, 0x25, 0x15, 0x02, 0x03, 0x01,
  0x00, 0x01, 0xa3, 0x81, 0xb2, 0x30, 0x81, 0xaf, 0x30, 0x0f, 0x06, 0x03,
  0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01,
  0xff, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04,
  0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x6d, 0x06, 0x08, 0x2b, 0x06, 0x01,
  0x05, 0x05, 0x07, 0x01, 0x0c, 0x04, 0x61, 0x30, 0x5f, 0xa1, 0x5d, 0xa0,
  0x5b, 0x30, 0x59, 0x30, 0x57, 0x30, 0x55, 0x16, 0x09, 0x69, 0x6d, 0x61,
  0x67, 0x65, 0x2f, 0x67, 0x69, 0x66, 0x30, 0x21, 0x30, 0x1f, 0x30, 0x07,
  0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x04, 0x14, 0x8f, 0xe5, 0xd3,
  0x1a, 0x86, 0xac, 0x8d, 0x8e, 0x6b, 0xc3, 0xcf, 0x80, 0x6a, 0xd4, 0x48,
  0x18, 0x2c, 0x7b, 0x19, 0x2e, 0x30, 0x25, 0x16, 0x23, 0x68, 0x74, 0x74,
  0x70, 0x3a, 0x2f, 0x2f, 0x6c, 0x6f, 0x67, 0x6f, 0x2e, 0x76, 0x65, 0x72,
  0x69, 0x73, 0x69, 0x67, 0x6e, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x76, 0x73,
  0x6c, 0x6f, 0x67, 0x6f, 0x2e, 0x67, 0x69, 0x66, 0x30, 0x1d, 0x06, 0x03,
  0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x7f, 0xd3, 0x65, 0xa7, 0xc2,
  0xdd, 0xec, 0xbb, 0xf0, 0x30, 0x09, 0xf3, 0x43, 0x39, 0xfa, 0x02, 0xaf,
  0x33, 0x31, 0x33, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
  0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x93,
  0x24, 0x4a, 0x30, 0x5f, 0x62, 0xcf, 0xd8, 0x1a, 0x98, 0x2f, 0x3d, 0xea,
  0xdc, 0x99, 0x2d, 0xbd, 0x77, 0xf6, 0xa5, 0x79, 0x22, 0x38, 0xec, 0xc4,
  0xa7, 0xa0, 0x78, 0x12, 0xad, 0x62, 0x0e, 0x45, 0x70, 0x64, 0xc5, 0xe7,
  0x97, 0x66, 0x2d, 0x98, 0x09, 0x7e, 0x5f, 0xaf, 0xd6, 0xcc, 0x28, 0x65,
  0xf2, 0x01, 0xaa, 0x08, 0x1a, 0x47, 0xde, 0xf9, 0xf9, 0x7c, 0x92, 0x5a,
  0x08, 0x69, 0x20, 0x0d, 0xd9, 0x3e, 0x6d, 0x6e, 0x3c, 0x0d, 0x6e, 0xd8,
  0xe6, 0x06, 0x91, 0x40, 0x18, 0xb9, 0xf8, 0xc1, 0xed, 0xdf, 0xdb, 0x41,
  0xaa, 0xe0, 0x96, 0x20, 0xc9, 0xcd, 0x64, 0x15, 0x38, 0x81, 0xc9, 0x94,
  0xee, 0xa2, 0x84, 0x29, 0x0b, 0x13, 0x6f, 0x8e, 0xdb, 0x0c, 0xdd, 0x25,
  0x02, 0xdb, 0xa4, 0x8b, 0x19, 0x44, 0xd2, 0x41, 0x7a, 0x05, 0x69, 0x4a,
  0x58, 0x4f, 0x60, 0xca, 0x7e, 0x82, 0x6a, 0x0b, 0x02, 0xaa, 0x25, 0x17,
  0x39, 0xb5, 0xdb, 0x7f, 0xe7, 0x84, 0x65, 0x2a, 0x95, 0x8a, 0xbd, 0x86,
  0xde, 0x5e, 0x81, 0x16, 0x83, 0x2d, 0x10, 0xcc, 0xde, 0xfd, 0xa8, 0x82,
  0x2a, 0x6d, 0x28, 0x1f, 0x0d, 0x0b, 0xc4, 0xe5, 0xe7, 0x1a, 0x26, 0x19,
  0xe1, 0xf4, 0x11, 0x6f, 0x10, 0xb5, 0x95, 0xfc, 0xe7, 0x42, 0x05, 0x32,
  0xdb, 0xce, 0x9d, 0x51, 0x5e, 0x28, 0xb6, 0x9e, 0x85, 0xd3, 0x5b, 0xef,
  0xa5, 0x7d, 0x45, 0x40, 0x72, 0x8e, 0xb7, 0x0e, 0x6b, 0x0e, 0x06, 0xfb,
  0x33, 0x35, 0x48, 0x71, 0xb8, 0x9d, 0x27, 0x8b, 0xc4, 0x65, 0x5f, 0x0d,
  0x86, 0x76, 0x9c, 0x44, 0x7a, 0xf6, 0x95, 0x5c, 0xf6, 0x5d, 0x32, 0x08,
  0x33, 0xa4, 0x54, 0xb6, 0x18, 0x3f, 0x68, 0x5c, 0xf2, 0x42, 0x4a, 0x85,
  0x38, 0x54, 0x83, 0x5f, 0xd1, 0xe8, 0x2c, 0xf2, 0xac, 0x11, 0xd6, 0xa8,
  0xed, 0x63, 0x6a
};
unsigned int certificates_esp8266_bin_CA_len = 1239;
unsigned char certificates_esp8266_bin_key[] =
{
  0x30, 0x82, 0x04, 0xa3, 0x02, 0x01, 0x00, 0x02, 0x82, 0x01, 0x01, 0x00,
  0xac, 0x5c, 0x9d, 0x8d, 0x04, 0x54, 0xbd, 0xe3, 0xdb, 0x04, 0xe8, 0x76,
  0x92, 0x9d, 0xf0, 0x66, 0xfa, 0x51, 0x3c, 0xdd, 0x64, 0x0c, 0xaa, 0xb0,
  0x4e, 0x82, 0x44, 0x69, 0x03, 0x61, 0xc7, 0x36, 0x25, 0x48, 0xf5, 0xc3,
  0xf8, 0x51, 0xf7, 0xf3, 0xd5, 0xc0, 0xd9, 0xb3, 0xc5, 0x38, 0x0b, 0x35,
  0x2b, 0xbd, 0x1e, 0x0a, 0x67, 0x08, 0x00, 0xe0, 0x25, 0xf8, 0xbd, 0x79,
  0xd0, 0x6c, 0x40, 0xaa, 0x5a, 0x9c, 0x63, 0x2c, 0xf4, 0xcb, 0x10, 0x3d,
  0x65, 0xca, 0x31, 0x91, 0xed, 0xd3, 0x35, 0x65, 0x53, 0x06, 0x31, 0xc2,
  0x74, 0xb1, 0xd8, 0x01, 0xa6, 0xff, 0x7b, 0x7d, 0x5e, 0x5a, 0x80, 0x84,
  0x27, 0xb5, 0xca, 0xa4, 0xba, 0x0f, 0xb6, 0xca, 0x1e, 0x0a, 0xb4, 0x2c,
  0x0a, 0x85, 0xd1, 0x72, 0x54, 0x44, 0xd5, 0x7c, 0x28, 0x55, 0xba, 0xbf,
  0x2a, 0x34, 0x2b, 0x20, 0x3c, 0x19, 0xd3, 0x22, 0x16, 0xf0, 0x46, 0x67,
  0x32, 0x97, 0x94, 0xcc, 0x63, 0x5f, 0x9f, 0xd1, 0x5f, 0xd3, 0xdd, 0xf0,
  0xb7, 0xe2, 0xae, 0xbb, 0x47, 0x76, 0x5a, 0xb8, 0x82, 0xb5, 0x2c, 0x93,
  0xa2, 0x19, 0xd2, 0x5a, 0xcd, 0x7c, 0x3d, 0xa1, 0x58, 0x1f, 0xfa, 0xa9,
  0x72, 0xb1, 0x05, 0x10, 0xf1, 0x4c, 0x8a, 0x76, 0x39, 0x85, 0xf6, 0xc6,
  0xdf, 0x02, 0x29, 0xa7, 0xe5, 0x85, 0x73, 0xa7, 0x7b, 0xd3, 0x4a, 0x2a,
  0x2b, 0xc0, 0xbd, 0x9b, 0x03, 0xc6, 0xe7, 0x55, 0xca, 0x60, 0x8a, 0x2a,
  0xf0, 0x28, 0x4f, 0x5a, 0xe9, 0xc1, 0x69, 0xfd, 0xd9, 0xd6, 0x90, 0xf7,
  0xc4, 0x24, 0x69, 0x22, 0xf7, 0xd4, 0xe7, 0x88, 0xdb, 0x95, 0x6b, 0xb5,
  0xc0, 0x16, 0x2d, 0x6b, 0x57, 0xb1, 0xe8, 0x85, 0xaf, 0x0e, 0x93, 0xf1,
  0x97, 0xce, 0x01, 0x87, 0x06, 0x52, 0xbf, 0x82, 0xf1, 0x59, 0x23, 0x34,
  0x45, 0xb7, 0xb6, 0xc1, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x82, 0x01,
  0x00, 0x1e, 0xef, 0xc4, 0xab, 0x91, 0xf2, 0x6f, 0xde, 0xdc, 0xe9, 0xe5,
  0x9b, 0x9a, 0xb7, 0x54, 0x75, 0xf7, 0x49, 0xeb, 0x85, 0xc5, 0x36, 0x6c,
  0xd0, 0x96, 0x56, 0xd5, 0x56, 0x8f, 0xef, 0xd5, 0xe4, 0xc7, 0x0e, 0x70,
  0x66, 0x4b, 0x44, 0xd3, 0x1f, 0x65, 0xe7, 0x66, 0x6b, 0xd8, 0xe3, 0x35,
  0x47, 0xc6, 0xe0, 0xfb, 0xea, 0x5c, 0x11, 0xd8, 0xae, 0x62, 0xc2, 0x61,
  0xa7, 0xe2, 0x9b, 0xfb, 0x83, 0x58, 0x1e, 0x27, 0xc2, 0xe2, 0xf1, 0x2b,
  0xaa, 0x09, 0x82, 0x20, 0x21, 0x94, 0x31, 0x4b, 0xad, 0x33, 0x18, 0xb2,
  0x87, 0xb7, 0x90, 0x50, 0x38, 0x03, 0xb4, 0x69, 0x82, 0xd8, 0x13, 0xa0,
  0x6b, 0x98, 0x01, 0x0e, 0x2d, 0x40, 0xe4, 0x1f, 0x27, 0xe9, 0xfa, 0x7b,
  0xda, 0x47, 0xcd, 0x3b, 0x09, 0xad, 0x4c, 0x69, 0x91, 0x0e, 0x60, 0x4c,
  0xd3, 0x91, 0x0c, 0xaa, 0x0c, 0xf0, 0x1c, 0xa6, 0x83, 0x75, 0x45, 0x9b,
  0xca, 0x52, 0x52, 0x1b, 0x63, 0xa8, 0x9b, 0xf5, 0x2e, 0x24, 0xcf, 0xa0,
  0x18, 0x1d, 0xff, 0x88, 0x64, 0x25, 0x71, 0x20, 0x5f, 0x32, 0xe6, 0xb7,
  0x00, 0xd2, 0xbe, 0x77, 0x3d, 0x05, 0xaf, 0xd3, 0x54, 0xee, 0x56, 0x24,
  0x4c, 0x82, 0x04, 0xd7, 0x32, 0xdd, 0x55, 0xa6, 0x46, 0xea, 0x12, 0xe1,
  0x33, 0x66, 0x8f, 0x48, 0xf6, 0x16, 0x09, 0x9b, 0x0d, 0x02, 0xd6, 0x8c,
  0x3e, 0x6f, 0x08, 0x1f, 0x3e, 0x55, 0xc1, 0x6c, 0x31, 0xc9, 0x18, 0x84,
  0xfe, 0xf3, 0xfc, 0xed, 0x14, 0xbd, 0xaa, 0xb9, 0x0b, 0xdd, 0xe1, 0x84,
  0x98, 0x93, 0x82, 0xc7, 0xc0, 0x53, 0x85, 0x5d, 0x0f, 0x71, 0xdb, 0x20,
  0x0b, 0x10, 0x4f, 0x04, 0xac, 0x43, 0x47, 0x93, 0x5c, 0x03, 0xa9, 0xf6,
  0x28, 0xa0, 0x6e, 0xcd, 0x6c, 0x29, 0x32, 0xb6, 0x37, 0xf6, 0x3f, 0x4f,
  0x11, 0x28, 0x44, 0x25, 0x41, 0x02, 0x81, 0x81, 0x00, 0xe1, 0xdf, 0xdc,
  0x85, 0xc4, 0xee, 0x7b, 0x5f, 0x16, 0x23, 0x3c, 0x8c, 0x74, 0x4d, 0x22,
  0xa0, 0x5a, 0x2c, 0xbe, 0x01, 0x3b, 0x0c, 0xe5, 0x14, 0x58, 0xf7, 0xf9,
  0x2f, 0x95, 0x2b, 0x06, 0x98, 0xd1, 0x31, 0xf1, 0x36, 0xa9, 0xbb, 0xe4,
  0x2a, 0x09, 0xb6, 0x73, 0x75, 0x77, 0xa1, 0x7f, 0xbd, 0x0a, 0x94, 0xc1,
  0x35, 0x6c, 0xaa, 0x90, 0x14, 0x91, 0xe6, 0xef, 0xe2, 0xf6, 0xce, 0x01,
  0x01, 0xd2, 0x3d, 0x92, 0x0e, 0x2a, 0xf8, 0x20, 0xf0, 0xb0, 0xfc, 0x56,
  0xd7, 0xc8, 0x9c, 0x64, 0x53, 0xf6, 0x6c, 0x8c, 0x99, 0x12, 0x9d, 0x66,
  0x58, 0x67, 0x1d, 0x8d, 0x10, 0x88, 0xf0, 0x41, 0x03, 0x98, 0x29, 0xad,
  0xa2, 0xb6, 0x3f, 0x05, 0xbf, 0xeb, 0xa1, 0xb3, 0x21, 0x75, 0xa3, 0xa4,
  0x7c, 0xa0, 0xb7, 0x4d, 0xc5, 0x29, 0x2d, 0x67, 0x6a, 0x18, 0x47, 0xf7,
  0x17, 0x87, 0xf8, 0xac, 0xe9, 0x02, 0x81, 0x81, 0x00, 0xc3, 0x59, 0xa5,
  0x2f, 0xcb, 0x89, 0x97, 0xb3, 0xaf, 0x0c, 0x40, 0x93, 0x98, 0x11, 0xd2,
  0x74, 0x2a, 0x18, 0x9b, 0x23, 0xa5, 0x9e, 0x5e, 0xac, 0x8e, 0xc3, 0xf8,
  0x9e, 0x5c, 0x09, 0x1b, 0xa3, 0xc2, 0x13, 0xbc, 0xe4, 0xdd, 0xe5, 0xfa,
  0x73, 0xde, 0x62, 0x28, 0x1b, 0x20, 0x57, 0x48, 0xb1, 0x4c, 0x27, 0x21,
  0xe3, 0x5e, 0x4e, 0x1f, 0x20, 0xbe, 0x5f, 0xca, 0x97, 0xe2, 0xc3, 0x9f,
  0xe2, 0x5a, 0x75, 0x36, 0xc5, 0x20, 0x5e, 0xc8, 0x6a, 0x13, 0x9e, 0x16,
  0xa7, 0x07, 0xb0, 0x1b, 0xd3, 0xd0, 0x05, 0x85, 0xc8, 0xbc, 0xeb, 0xc4,
  0x2e, 0x69, 0x08, 0x44, 0xc0, 0x1e, 0x76, 0x9a, 0x72, 0x38, 0xf0, 0xbd,
  0x43, 0xd8, 0x75, 0x43, 0xc2, 0x7d, 0xfa, 0x58, 0x2d, 0x8e, 0x02, 0xfa,
  0x05, 0x5f, 0x63, 0xe0, 0xa2, 0x3a, 0x7d, 0xa6, 0x62, 0xe0, 0x43, 0xc9,
  0x5c, 0xd9, 0x23, 0xb4, 0x19, 0x02, 0x81, 0x80, 0x6a, 0x81, 0xfb, 0xe3,
  0x5c, 0x0c, 0x34, 0x42, 0x8f, 0x16, 0xc8, 0x7b, 0x56, 0x6f, 0xf4, 0xee,
  0xf5, 0xf6, 0x79, 0x15, 0xed, 0x79, 0x54, 0x1f, 0xf7, 0x1f, 0xde, 0xf1,
  0x8f, 0xfe, 0x3c, 0xdb, 0x1c, 0x71, 0xd7, 0x53, 0x36, 0x50, 0x31, 0xf3,
  0x9c, 0xa6, 0xda, 0xe4, 0xbe, 0x27, 0x7a, 0x2b, 0x72, 0xec, 0xd7, 0xfa,
  0x50, 0x26, 0x51, 0x50, 0x51, 0xc8, 0x3a, 0xbf, 0xa4, 0x2f, 0x59, 0x8c,
  0x39, 0x4d, 0xed, 0x2c, 0x1f, 0x18, 0xe7, 0xb7, 0x77, 0x4d, 0x55, 0xc9,
  0xe4, 0x7f, 0xd2, 0x53, 0x41, 0x34, 0xc8, 0x4b, 0x79, 0xda, 0x9c, 0xa2,
  0xfc, 0xe4, 0x93, 0x5b, 0x03, 0xa7, 0x7e, 0x90, 0xaa, 0x54, 0xa4, 0xda,
  0xbf, 0xe3, 0x11, 0xdc, 0x00, 0xc9, 0x3b, 0xc5, 0x80, 0x4a, 0x4c, 0xca,
  0xb3, 0xa2, 0x1b, 0xda, 0xe7, 0x62, 0xd0, 0x8b, 0xd7, 0xad, 0x3f, 0x97,
  0x69, 0xf7, 0x59, 0xd9, 0x02, 0x81, 0x81, 0x00, 0xbb, 0xef, 0x1d, 0x5c,
  0xea, 0xda, 0x57, 0x28, 0x95, 0x4a, 0x1b, 0x42, 0x22, 0xf5, 0x5a, 0x5b,
  0x49, 0x38, 0x5c, 0xe0, 0x21, 0x70, 0x47, 0xb2, 0x88, 0xb2, 0x7e, 0x17,
  0x93, 0x81, 0x6b, 0xa6, 0xd2, 0x8a, 0x7c, 0x9a, 0x4d, 0xe3, 0xf7, 0x5b,
  0xcb, 0x60, 0xf1, 0x26, 0x8f, 0xa9, 0x89, 0x14, 0x90, 0xa7, 0xc1, 0x11,
  0x65, 0x63, 0x90, 0xcc, 0xc6, 0x79, 0x8f, 0x85, 0x29, 0xb2, 0x7b, 0xb9,
  0x9e, 0xbe, 0xcf, 0x4f, 0x3c, 0x02, 0x13, 0x9b, 0x01, 0x7e, 0x80, 0x5a,
  0x76, 0xa9, 0xf5, 0x4b, 0x84, 0x00, 0x1c, 0x21, 0xef, 0xa6, 0x7e, 0x19,
  0x0c, 0x10, 0xd9, 0x5b, 0x5e, 0xb3, 0x49, 0x88, 0x83, 0xc1, 0x0e, 0xc4,
  0x1b, 0x95, 0x4c, 0xf7, 0xea, 0x8c, 0x92, 0x92, 0x58, 0xe2, 0x02, 0x38,
  0x72, 0xf9, 0xba, 0x5d, 0x38, 0x1a, 0x33, 0x9d, 0x42, 0x15, 0xd4, 0xbd,
  0xcf, 0x64, 0x6a, 0xd9, 0x02, 0x81, 0x80, 0x17, 0x1b, 0x17, 0x17, 0x98,
  0x66, 0x7c, 0x8d, 0xe4, 0x5b, 0x4b, 0x3d, 0xd5, 0x18, 0x51, 0x30, 0x8d,
  0x4c, 0xa0, 0x2f, 0xc7, 0xd9, 0x62, 0x4a, 0xc9, 0x9a, 0x9b, 0xf6, 0x88,
  0xae, 0xbd, 0xae, 0xe6, 0x13, 0x73, 0x6e, 0xe1, 0x99, 0x61, 0x15, 0x37,
  0xde, 0x02, 0x36, 0xda, 0xb8, 0x93, 0x88, 0xd3, 0x86, 0x96, 0x8a, 0xaf,
  0x9b, 0xcd, 0x34, 0xf5, 0xcd, 0x6a, 0x2e, 0x29, 0xa6, 0xdb, 0x34, 0x60,
  0xf7, 0xe0, 0x43, 0xad, 0x1e, 0xd2, 0x99, 0xd4, 0xfe, 0x45, 0x98, 0x68,
  0x33, 0x25, 0xdf, 0x61, 0x4a, 0xea, 0x42, 0xc8, 0xf2, 0x30, 0x75, 0x51,
  0x3b, 0x20, 0xd9, 0xb5, 0x81, 0x4b, 0xe0, 0x60, 0xe6, 0xe9, 0x35, 0x66,
  0x15, 0x58, 0x26, 0x46, 0x27, 0x59, 0x6d, 0x59, 0x80, 0x3b, 0x40, 0x64,
  0x88, 0xf8, 0xa0, 0x0c, 0xa0, 0xc9, 0xa2, 0x23, 0xb9, 0x99, 0x21, 0x0d,
  0xc9, 0x0d, 0x55
};
unsigned int certificates_esp8266_bin_key_len = 1191;

unsigned char certificates_esp8266_bin_crt[] =
{
  0x30, 0x82, 0x03, 0x59, 0x30, 0x82, 0x02, 0x41, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x14, 0x0f, 0x8a, 0x21, 0xb1, 0x22, 0xf8, 0x60, 0x0f, 0x52,
  0xe9, 0xdf, 0xe6, 0x8c, 0xdb, 0x4f, 0x85, 0x18, 0x85, 0xe8, 0x44, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
  0x05, 0x00, 0x30, 0x4d, 0x31, 0x4b, 0x30, 0x49, 0x06, 0x03, 0x55, 0x04,
  0x0b, 0x0c, 0x42, 0x41, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x20, 0x57, 0x65,
  0x62, 0x20, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x73, 0x20, 0x4f,
  0x3d, 0x41, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x2e, 0x63, 0x6f, 0x6d, 0x20,
  0x49, 0x6e, 0x63, 0x2e, 0x20, 0x4c, 0x3d, 0x53, 0x65, 0x61, 0x74, 0x74,
  0x6c, 0x65, 0x20, 0x53, 0x54, 0x3d, 0x57, 0x61, 0x73, 0x68, 0x69, 0x6e,
  0x67, 0x74, 0x6f, 0x6e, 0x20, 0x43, 0x3d, 0x55, 0x53, 0x30, 0x1e, 0x17,
  0x0d, 0x31, 0x38, 0x30, 0x33, 0x33, 0x31, 0x30, 0x38, 0x31, 0x30, 0x31,
  0x30, 0x5a, 0x17, 0x0d, 0x34, 0x39, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33,
  0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x1e, 0x31, 0x1c, 0x30, 0x1a, 0x06,
  0x03, 0x55, 0x04, 0x03, 0x0c, 0x13, 0x41, 0x57, 0x53, 0x20, 0x49, 0x6f,
  0x54, 0x20, 0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74,
  0x65, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48,
  0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f,
  0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xac, 0x5c,
  0x9d, 0x8d, 0x04, 0x54, 0xbd, 0xe3, 0xdb, 0x04, 0xe8, 0x76, 0x92, 0x9d,
  0xf0, 0x66, 0xfa, 0x51, 0x3c, 0xdd, 0x64, 0x0c, 0xaa, 0xb0, 0x4e, 0x82,
  0x44, 0x69, 0x03, 0x61, 0xc7, 0x36, 0x25, 0x48, 0xf5, 0xc3, 0xf8, 0x51,
  0xf7, 0xf3, 0xd5, 0xc0, 0xd9, 0xb3, 0xc5, 0x38, 0x0b, 0x35, 0x2b, 0xbd,
  0x1e, 0x0a, 0x67, 0x08, 0x00, 0xe0, 0x25, 0xf8, 0xbd, 0x79, 0xd0, 0x6c,
  0x40, 0xaa, 0x5a, 0x9c, 0x63, 0x2c, 0xf4, 0xcb, 0x10, 0x3d, 0x65, 0xca,
  0x31, 0x91, 0xed, 0xd3, 0x35, 0x65, 0x53, 0x06, 0x31, 0xc2, 0x74, 0xb1,
  0xd8, 0x01, 0xa6, 0xff, 0x7b, 0x7d, 0x5e, 0x5a, 0x80, 0x84, 0x27, 0xb5,
  0xca, 0xa4, 0xba, 0x0f, 0xb6, 0xca, 0x1e, 0x0a, 0xb4, 0x2c, 0x0a, 0x85,
  0xd1, 0x72, 0x54, 0x44, 0xd5, 0x7c, 0x28, 0x55, 0xba, 0xbf, 0x2a, 0x34,
  0x2b, 0x20, 0x3c, 0x19, 0xd3, 0x22, 0x16, 0xf0, 0x46, 0x67, 0x32, 0x97,
  0x94, 0xcc, 0x63, 0x5f, 0x9f, 0xd1, 0x5f, 0xd3, 0xdd, 0xf0, 0xb7, 0xe2,
  0xae, 0xbb, 0x47, 0x76, 0x5a, 0xb8, 0x82, 0xb5, 0x2c, 0x93, 0xa2, 0x19,
  0xd2, 0x5a, 0xcd, 0x7c, 0x3d, 0xa1, 0x58, 0x1f, 0xfa, 0xa9, 0x72, 0xb1,
  0x05, 0x10, 0xf1, 0x4c, 0x8a, 0x76, 0x39, 0x85, 0xf6, 0xc6, 0xdf, 0x02,
  0x29, 0xa7, 0xe5, 0x85, 0x73, 0xa7, 0x7b, 0xd3, 0x4a, 0x2a, 0x2b, 0xc0,
  0xbd, 0x9b, 0x03, 0xc6, 0xe7, 0x55, 0xca, 0x60, 0x8a, 0x2a, 0xf0, 0x28,
  0x4f, 0x5a, 0xe9, 0xc1, 0x69, 0xfd, 0xd9, 0xd6, 0x90, 0xf7, 0xc4, 0x24,
  0x69, 0x22, 0xf7, 0xd4, 0xe7, 0x88, 0xdb, 0x95, 0x6b, 0xb5, 0xc0, 0x16,
  0x2d, 0x6b, 0x57, 0xb1, 0xe8, 0x85, 0xaf, 0x0e, 0x93, 0xf1, 0x97, 0xce,
  0x01, 0x87, 0x06, 0x52, 0xbf, 0x82, 0xf1, 0x59, 0x23, 0x34, 0x45, 0xb7,
  0xb6, 0xc1, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x60, 0x30, 0x5e, 0x30,
  0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14,
  0xa5, 0x0f, 0xb4, 0xca, 0xdd, 0xa2, 0x36, 0x14, 0xfd, 0x42, 0x8a, 0x6c,
  0xf9, 0x88, 0xf3, 0xd2, 0x16, 0x9b, 0x51, 0x97, 0x30, 0x1d, 0x06, 0x03,
  0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xba, 0xdd, 0xf0, 0xdf, 0x60,
  0x9f, 0xc2, 0x38, 0x04, 0x15, 0xe3, 0xc9, 0x09, 0x28, 0x90, 0x2a, 0xb2,
  0xa5, 0x06, 0x26, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01,
  0xff, 0x04, 0x02, 0x30, 0x00, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f,
  0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x07, 0x80, 0x30, 0x0d, 0x06,
  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00,
  0x03, 0x82, 0x01, 0x01, 0x00, 0x9b, 0xee, 0x82, 0xf6, 0xfd, 0x23, 0x88,
  0x49, 0xa5, 0x37, 0x7b, 0x2b, 0xc2, 0xa1, 0x60, 0xc6, 0x5c, 0xd5, 0xa5,
  0xd4, 0xea, 0x62, 0x0d, 0x48, 0x2f, 0xbf, 0xbf, 0x99, 0x87, 0xc8, 0x42,
  0x72, 0xae, 0xd5, 0xcf, 0xbd, 0x66, 0xea, 0x83, 0xfc, 0x0a, 0xbf, 0xb7,
  0x46, 0x85, 0x2f, 0xd8, 0x8c, 0x22, 0xf7, 0x95, 0x22, 0x8b, 0x2e, 0x79,
  0xdd, 0xbc, 0x3e, 0x88, 0xd3, 0xab, 0x48, 0x9e, 0x69, 0x83, 0x25, 0xf2,
  0xc1, 0x2c, 0xd2, 0xab, 0xd6, 0xd1, 0x19, 0x68, 0x82, 0x1e, 0xad, 0xb6,
  0xd1, 0x31, 0x2c, 0xe1, 0xcf, 0x3d, 0xbf, 0x4d, 0x23, 0x37, 0x18, 0x3c,
  0xa1, 0xf9, 0xf1, 0x44, 0x11, 0x56, 0x90, 0x21, 0x93, 0xfc, 0xe8, 0x55,
  0x13, 0xe7, 0x6e, 0x51, 0x8f, 0xb7, 0x38, 0x3e, 0x0e, 0x12, 0xd1, 0xb0,
  0x7c, 0x60, 0x9c, 0x36, 0x4b, 0xef, 0x68, 0x4a, 0x49, 0x0e, 0x70, 0xf5,
  0x27, 0x68, 0x46, 0xc8, 0x59, 0x5a, 0x55, 0x2a, 0x8e, 0xf3, 0xe3, 0x6e,
  0xde, 0xf6, 0x0a, 0xc5, 0x1d, 0x85, 0x1c, 0xab, 0x07, 0x23, 0xc0, 0x91,
  0xf0, 0xd6, 0x94, 0x06, 0x18, 0xc1, 0xa7, 0xec, 0x1a, 0xc9, 0x90, 0x98,
  0xb4, 0x3c, 0xb9, 0x9c, 0x5e, 0x25, 0xb6, 0x6a, 0xcc, 0xce, 0x2c, 0x6e,
  0x82, 0x0f, 0x24, 0xfc, 0x3f, 0x08, 0xc0, 0x8e, 0xd3, 0x95, 0x04, 0xe6,
  0x6b, 0x0f, 0x4e, 0xed, 0x4e, 0xfc, 0xd4, 0x7c, 0x29, 0x87, 0xeb, 0x15,
  0x0f, 0xb4, 0x16, 0x04, 0xd4, 0x38, 0xe1, 0xce, 0xbe, 0xbf, 0xc4, 0x5e,
  0xa8, 0x0e, 0xbe, 0x74, 0x2d, 0x83, 0x1b, 0x2a, 0xb9, 0x29, 0xcc, 0xb0,
  0xb4, 0xf2, 0xf0, 0x59, 0xb2, 0x0c, 0x7b, 0x1d, 0x81, 0x2a, 0xe9, 0xef,
  0x07, 0xb0, 0x60, 0xdc, 0xee, 0xd7, 0x5e, 0x14, 0x62, 0x0f, 0x73, 0x25,
  0xbb, 0x24, 0x4b, 0x22, 0x2d, 0xb9, 0x7e, 0xe0, 0x46
};
unsigned int certificates_esp8266_bin_crt_len = 861;
float t =0.0 ; //temperature
int gpio0_pin = 12; // Digital output 0
int gpio1_pin = 13; // Digital output 1
String recieved;

const char* AWS_endpoint = "a1t7wpltysp7gc.iot.us-east-2.amazonaws.com"; //MQTT broker ip
const char* ssid = "123";
const char* password = "1234567890";


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
    recieved += String((char)payload[i]);
  }
    if(recieved=="l01"){
      digitalWrite(gpio0_pin, HIGH);
      recieved="";
    }
    else if(recieved=="l00"){
      digitalWrite(gpio0_pin, LOW);
      recieved="";
    }
    else if(recieved=="l10"){
      digitalWrite(gpio1_pin, LOW);
      recieved="";
    }
    else if(recieved=="l11"){
      digitalWrite(gpio1_pin, HIGH);
      recieved="";
    }
  Serial.println();
}


WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient); //set  MQTT port number to 8883
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(gpio0_pin, OUTPUT);
  digitalWrite(gpio0_pin, LOW);
  pinMode(gpio1_pin, OUTPUT);
  digitalWrite(gpio1_pin, LOW);
  Serial.begin (115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  espClient.setCertificate(certificates_esp8266_bin_crt, certificates_esp8266_bin_crt_len);
  espClient.setPrivateKey(certificates_esp8266_bin_key, certificates_esp8266_bin_key_len);
  espClient.setCACert(certificates_esp8266_bin_CA, certificates_esp8266_bin_CA_len);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("a3");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    ++value;
    t = analogRead(A0)*0.25;
    snprintf (msg, 75, "{\"a3_lightIntensity\": \"%lf\"}", t);
    Serial.print("Iot 3 Publish light intensity: ");
    Serial.println(msg);
    client.publish("temp_read", msg);
	  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap()); //Low heap can cause problems
  }
}

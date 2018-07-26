const uint8_t FLAC_SHORT_HEADER[] = {
    0x66, 0x4C, 0x61, 0x43, 0x80, 0x00, 0x00, 0x22, 0x10, 0x00, 0x10,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x31, 0x97, 0x0A, 0xC4, 0x42, 0xF0,
    0x00, 0x8A, 0x48, 0x96, 0x45, 0x61, 0x31, 0x02, 0x8B, 0xFB, 0x21,
    0xE5, 0x5F, 0xFB, 0x6E, 0xDF, 0x48, 0xCE, 0x9F, 0xAE};

const uint8_t FLAC_HEADER2[] = {
    0x66, 0x4C, 0x61, 0x43, 0x80, 0x00, 0x00, 0x22, 0x04, 0x80, 0x04,
    0x80, 0x00, 0x00, 0x0E, 0x00, 0x0E, 0x7A, 0x0B, 0xB8, 0x02, 0xF0,
    0x00, 0xE4, 0x19, 0x9A, 0x9D, 0xF9, 0x55, 0x1B, 0x97, 0x55, 0x78,
    0x60, 0x54, 0x22, 0x4B, 0xCC, 0xF9, 0x37, 0xF3, 0xF1};

const uint8_t FLAC_LONG_HEADER[] = {
    0x66, 0x4C, 0x61, 0x43, 0x00, 0x00, 0x00, 0x22, 0x12, 0x00, 0x12, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xC4, 0x42, 0xF0, 0x00, 0xD4,
    0xA3, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xBA, 0xA8,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0D, 0x75, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x2F, 0xF8,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1A, 0xEA, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xA5, 0x48,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x28, 0x5F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2F, 0x1A, 0x98,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x35, 0xD5, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x8F, 0xE8,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x43, 0x4A, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4A, 0x05, 0x38,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x50, 0xBF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x7A, 0x88,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x5E, 0x35, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0xEF, 0xD8,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x6B, 0xAA, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0x65, 0x28,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x79, 0x1F, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xDA, 0x78,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x86, 0x95, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8D, 0x4F, 0xC8,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x94, 0x0A, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9A, 0xC5, 0x18,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xA1, 0x7F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA8, 0x3A, 0x68,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xAE, 0xF5, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB5, 0xAF, 0xB8,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xBC, 0x6A, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x25, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xC9, 0xDF, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD0, 0x9A, 0x58,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00,
    0x00, 0xB1, 0x1F, 0x00, 0x00, 0x00, 0x47, 0x53, 0x74, 0x72, 0x65, 0x61,
    0x6D, 0x65, 0x72, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x64, 0x20,
    0x76, 0x6F, 0x72, 0x62, 0x69, 0x73, 0x63, 0x6F, 0x6D, 0x6D, 0x65, 0x6E,
    0x74, 0x07, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x41, 0x4C, 0x42,
    0x55, 0x4D, 0x3D, 0x45, 0x6E, 0x64, 0x6C, 0x65, 0x73, 0x73, 0x20, 0x46,
    0x6F, 0x72, 0x6D, 0x73, 0x20, 0x4D, 0x6F, 0x73, 0x74, 0x20, 0x42, 0x65,
    0x61, 0x75, 0x74, 0x69, 0x66, 0x75, 0x6C, 0x10, 0x00, 0x00, 0x00, 0x41,
    0x52, 0x54, 0x49, 0x53, 0x54, 0x3D, 0x4E, 0x69, 0x67, 0x68, 0x74, 0x77,
    0x69, 0x73, 0x68, 0x09, 0x00, 0x00, 0x00, 0x44, 0x41, 0x54, 0x45, 0x3D,
    0x32, 0x30, 0x31, 0x35, 0x0C, 0x00, 0x00, 0x00, 0x44, 0x49, 0x53, 0x43,
    0x4E, 0x55, 0x4D, 0x42, 0x45, 0x52, 0x3D, 0x31, 0x0B, 0x00, 0x00, 0x00,
    0x47, 0x45, 0x4E, 0x52, 0x45, 0x3D, 0x4D, 0x65, 0x74, 0x61, 0x6C, 0x0F,
    0x00, 0x00, 0x00, 0x54, 0x49, 0x54, 0x4C, 0x45, 0x3D, 0x45, 0x64, 0x65,
    0x6D, 0x61, 0x20, 0x52, 0x75, 0x68, 0x0D, 0x00, 0x00, 0x00, 0x54, 0x52,
    0x41, 0x43, 0x4B, 0x4E, 0x55, 0x4D, 0x42, 0x45, 0x52, 0x3D, 0x38};

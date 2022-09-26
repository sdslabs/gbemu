class GameHeader
{

private: // make all data members private
    union
    {
        uint8_t game_header[80];
        struct
        {
            uint8_t entry_point[4];
            uint8_t nintendo_logo[48];
            union
            {
                uint8_t title[16];
                struct
                {
                    uint8_t small_title[15];
                    uint8_t cgb_flag[1];
                };
                struct
                {
                    uint8_t tiny_title[11];
                    uint8_t manufacturer_code[4];
                    uint8_t cgb_flag_other[1];
                };
            };
            uint8_t new_license_code[2];
            uint8_t sgb_flag[1];
            uint8_t cartridge_type[1];
            uint8_t rom_size[1];
            uint8_t ram_size[1];
            uint8_t destination_code[1];
            uint8_t old_license_code[1];
            uint8_t mask_rom_version_number[1];
            uint8_t header_checksum[1];
            uint8_t global_checksum[2];
        };
    };

public:                                     // make public fetch and update functions
    void updateGameHeader(int i, uint8_t v) // update ith element of game_header to value v
    {
        game_header[i] = v;
    }

    uint8_t fetchGameHeader(int i) // fetch ith element of game_header
    {
        return game_header[i];
    }

    uint8_t fetchNintendoLogo(int i) // fetch ith element of nintendo_logo
    {
        return nintendo_logo[i];
    }

    uint8_t fetchHeaderChecksum(int i) // fetch ith element of header_checksum
    {
        return header_checksum[i];
    }

    uint8_t fetchTitle(int i) // fetch ith element of title
    {
        return title[i];
    }
};

uint8_t original_nintendo_logo[48] = {206, 237, 102, 102, 204, 13, 0, 11, 3, 115, 0, 131, 0, 12, 0, 13, 0, 8, 17, 31, 136, 137, 0, 14, 220, 204, 110, 230, 221, 221, 217, 153, 187, 187, 103, 99, 110, 14, 236, 204, 221, 220, 153, 159, 187, 185, 51, 62}; // Original Nintendo logo
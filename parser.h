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
};
import argparse
import numpy as np

# Note: file is read as int16's so offsets in code are in 16 bit steps
#   But the hexdump output indices are in bytes - so we are doing conversions all over here
#   All of the 'next's are offsets in bytes

def make_NGM_file_slice(offset_in_words, length_in_int16s):
    return (slice(offset_in_words, (offset_in_words + length_in_int16s)), offset_in_words + length_in_int16s)

def show_hex16(file, offset, name):
    print(name+': 0x%x' % file[offset])

def show_hex32(file, offset, name):
    bits_32 = get_32_bit_number(file, offset)
    print(name+': 0x%x' % bits_32)

def get_32_bit_number(file, offset):
    high_order_16 = file[(offset + 1)] << 16
    return (high_order_16 + file[offset])

def dump_hex(file_data, offset, count):
    print('0x%05x: ' % 0, end='')
    for i in np.arange(offset, offset+count):
        print('0x%04x ' % file_data[i], end='')
        if (i - offset + 1) % 16 == 0:
            print()
            print('0x%05x: ' % (i - offset + 1), end='')
    print()

def show_NGM_header(file_data, word_offset, NGM_header_count, short=False):
    print('\n' + 20*'#')
    print('===== NGM header #%d =====' % NGM_header_count)
    vhex = np.vectorize(hex)
    if not short:
        phdrid_slice, word_offset = make_NGM_file_slice(word_offset, 4)
        phdrid = file_data[phdrid_slice]
        print('phdrid: %s' % vhex(phdrid))

    hdrid_slice, word_offset = make_NGM_file_slice(word_offset, 2)
    hdrid = file_data[hdrid_slice]
    print('hdrid: %s' % vhex(hdrid))

    sliced, word_offset = make_NGM_file_slice(word_offset, 12)
    triggerstatspill = file_data[sliced]
    print('triggerstatspill: %s' % vhex(triggerstatspill))

    # This is the length of this block of reads before the next NGM header (in words) = 0x2244c = decimal 140364
    # For that value there are 28 3316 data blocks in one NGM block
    show_hex32(file_data, word_offset, 'databuffferread')
    word_offset += 2
    print(20*'='+ '\n')
    return word_offset

if __name__ == '__main__':
    parser = argparse.ArgumentParser('Parse NGM Struck File')
    parser.add_argument('file', help='NGM Struck file')
    _args = parser.parse_args()

    with open(_args.file, mode='rb') as file: # b is important -> binary
        #fileContent = file.read()                              # Read as bytes
        fileContent = np.fromfile(file, dtype=np.uint16)        # Read as int16s

    sliced, word_offset = make_NGM_file_slice(200, 20)
    abba = fileContent[sliced]    # The 0xabbas (these could be variable in length?)
    vhex = np.vectorize(hex)
    print('Magic block: %s' % vhex(abba))

    NGM_header_count = 1
    NGM_this_block_header_count = 0     # this number is immediately overridden below
    S3316_data_block_counter = 1

    while True:
        channel_and_format = fileContent[word_offset]
        if (channel_and_format & 0xF) != 0x05:          # This is not a good/reliable detection mechanism - need to use a length
            print('\n' + 20 * '!')
            print('===>>> Channel and format don''t match 0x05: 0x%x.  Must be a new NGM block' % channel_and_format)
            print('Number of 3316 blocks in previous NGM block: %d' % NGM_this_block_header_count)
            NGM_this_block_header_count = 1

            # Try the standard as a shortcut to avoud searching
            # TODO: however 14 is the most common header length
            lower_length_word = fileContent[word_offset + 18]  # 18 is the offset of the lower length word
            print('===>>> Lower length word try 0: 0x%x' % lower_length_word)
            if lower_length_word == 0x244c:  # Note that the first block is exempt from this test.  Which is probably not a good idea
                print('Length word offset: %d, header #: %d' % (18, NGM_header_count))
                word_offset = show_NGM_header(fileContent, word_offset, NGM_header_count)
            else:
                for i in np.arange(0, 0x100):
                    test_lower_length_word = fileContent[word_offset+i]
                    if test_lower_length_word == 0x244c:
                        break

                if i >= 0x100:
                    print('===>>> uh oh - Did not find the length word')        # Will blow up
                    dump_hex(fileContent, word_offset, 0x100)
                else:
                    length_offset = i
                    print('Length word offset: %d, header #: %d' % (length_offset, NGM_header_count))
                lower_length_word = fileContent[word_offset + i]
                print('===>>> Lower length word try 1: 0x%x' % lower_length_word)
                if lower_length_word != 0x244c:
                    print('===>>> uh-oh -- no length offset found')        # Will blow up
                    dump_hex(fileContent, word_offset, 0x100)
                else:
                    # In this case there is no pre-header, so reverse that offset
                    # And possibly the actual NGM header skips the phdr ID (see "short=True" below)
                    if length_offset == 14:
                        word_offset = show_NGM_header(fileContent, word_offset, NGM_header_count, short=True)
                    elif length_offset > 18:
                        extra_NGM_header_length = length_offset-18
                        print('Extra NGM header: %d' % extra_NGM_header_length)
                        word_offset += extra_NGM_header_length     # 18 is the default length ofset covered above
                        word_offset = show_NGM_header(fileContent, word_offset, NGM_header_count)
                    else:
                        print('===>>> uh-oh - unexpected length offset: %d' % length_offset)        # Will blow up
                        dump_hex(fileContent, word_offset, 0x100)
            NGM_header_count += 1
        else:
            NGM_this_block_header_count += 1

        print('===== 3316 data block #%d =====' % S3316_data_block_counter)
        if False:        # False is meant to suppress the prints for diagnostic, but the offsets wort work in that case
            # Display the 3316 Header
            show_hex16(fileContent, word_offset, 'channel and format')
            show_hex16(fileContent, word_offset+1, 'timestamp 1')
            show_hex16(fileContent, word_offset+2, 'timestamp 2')
            show_hex16(fileContent, word_offset+3, 'timestamp 3')
            show_hex16(fileContent, word_offset+4, 'peak high')
            show_hex16(fileContent, word_offset+5, 'index of peak high')
            show_hex16(fileContent, word_offset+6, 'information (upper 8 bits)')
            show_hex16(fileContent, word_offset+7, 'accumulator gate 1')
            show_hex32(fileContent, word_offset+8, 'accumulator gate 2')
            show_hex32(fileContent, word_offset+10, 'accumulator gate 3')
            show_hex32(fileContent, word_offset+12, 'accumulator gate 4')
            show_hex32(fileContent, word_offset+14, 'accumulator gate 5')
            show_hex32(fileContent, word_offset+16, 'accumulator gate 6')
            show_hex32(fileContent, word_offset+18, 'MAW max')
            show_hex32(fileContent, word_offset+20, 'MAW before trigger')
            show_hex32(fileContent, word_offset+22, 'MAW after trigger')
            word_offset += 24

            file_size_double_word = get_32_bit_number(fileContent, word_offset)
            show_hex32(fileContent, word_offset, 'Misc stuff - including sample count')
            size = file_size_double_word & 0x03FFFFFF
            print('Number of double words: 0x%x, %d' % (size, size))
            flag = (file_size_double_word & 0xF0000000) >> 28
            print('Flag: 0x%x' % (flag))
            word_offset += 2

            show_hex16(fileContent, word_offset + 1, 'First data word')
            show_hex16(fileContent, word_offset, 'Second data word')
            show_hex16(fileContent, word_offset + 3, 'Third data word')
            show_hex16(fileContent, word_offset + 2, 'Fourth data word')
            print('Next offset: 0x%x, %d' % (word_offset, word_offset))

        else:
            word_offset += 24
            file_size_double_word = get_32_bit_number(fileContent, word_offset) # Duplicative with above
            size = file_size_double_word & 0x03FFFFFF
            word_offset += 2
        word_offset += (2 * size)
        S3316_data_block_counter += 1


    # 0x000e says no average samples
    # next += 1
    # show_hex16(fileContent, next, 'Number of average samples')
    # next += 1
    # show_hex16(fileContent, next, 'Misc 2')


    initial_word_index = word_offset
    word_index = word_offset
    found_count = 0
    while found_count < 30:
        if (fileContent[word_index] & 0xFF00) != 0x5400:
            print('Offset, Data word: 0x%x: 0x%x' % (word_index, fileContent[word_index]))
            print('Relative offset (in words): 0x%x %d' % (word_index-initial_word_index, word_index-initial_word_index))
            found_count += 1
        word_index += 1


    print('Next offset: 0x%x' % word_offset)
    print('\n==========\n')


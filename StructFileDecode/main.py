import argparse
import numpy as np

# Note: file is read as int16's so offsets in code are in 16 bit steps
#   But the hexdump output indices are in bytes - so we are doing conversions all over here
#   All of the 'next's are offsets in bytes

# Offset in words and length in 16 bit? quantities (or is that really 8 = bytes) is confusing
def make_NGM_file_slice(offset_in_words, length_in_int16s):
    return slice(offset_in_words, (offset_in_words + length_in_int16s)), offset_in_words + length_in_int16s

def show_hex16(file, offset, name):
    print(name+': 0x%x' % file[offset])

def show_hex32(file, offset, name):
    bits_32 = get_32_bit_number(file, offset)
    print(name+': 0x%x' % bits_32)

def get_32_bit_number(file, offset):
    high_order_16 = file[(offset + 1)] << 16
    return (high_order_16 + file[offset])

def dump_hex(file_data, offset, count):
    print('==== DUMP OFFSET: 0x%x' % offset)
    print('0x%05x: ' % 0, end='')
    for i in np.arange(offset, offset+count):
        print('0x%04x ' % file_data[i], end='')
        if (i - offset + 1) % 16 == 0:
            print()
            print('0x%05x: ' % (i - offset + 1), end='')
    print()

def show_super_header(file_data, word_offset):
    #### Super header is the top level header 0xdc words long, and ends in the sequence 0xabba (x 20 repetitions)
    #       We dont know what the fields are in this header

    print('======= Super header =======')
    # Start of 0xabbas is at 0xc8 words
    sliced, word_offset = make_NGM_file_slice(0xc8, 20)
    abba = fileContent[sliced]    # The 0xabbas (these could be variable in length?)
    vhex = np.vectorize(hex)
    print('Magic block: %s' % vhex(abba))

    print('\n====  Word offset after first super header: 0x%x (in bytes: 0x%x) ====' % (word_offset, 2*word_offset)) # This is the end of the first super header
    print('======= End of super header =======\n')
    return word_offset

def show_NGM_header(file_data, word_offset, NGM_header_count, short=False, long=False):
    print('\n===== NGM header #%d =====' % NGM_header_count)
    vhex = np.vectorize(hex)
    size_of_NGM_header = 0 if not long else 12
    if not short:
        phdrid_slice, word_offset_ignore = make_NGM_file_slice(word_offset, 4)
        phdrid = file_data[phdrid_slice]
        print('phdrid: %s' % vhex(phdrid))
        size_of_NGM_header += 4

    hdrid_slice, word_offset_ignore = make_NGM_file_slice(word_offset + size_of_NGM_header, 2)
    hdrid = file_data[hdrid_slice]
    print('hdrid: %s' % vhex(hdrid))
    size_of_NGM_header += 2

    sliced, word_offset_ignore = make_NGM_file_slice(word_offset + size_of_NGM_header, 12)
    triggerstatspill = file_data[sliced]
    print('triggerstatspill: %s' % vhex(triggerstatspill))
    size_of_NGM_header += 12

    # This is the length of this block of reads before the next NGM header (in words) = 0x2244c = decimal 140364
    # For that value there are 28 3316 data blocks in one NGM block
    show_hex32(file_data, word_offset + size_of_NGM_header, 'databuffferread')
    size_of_NGM_header += 2
    print('===== End of NGM header #%d =====\n' % NGM_header_count)
    return size_of_NGM_header

def show_3316_header(fileContent, word_offset):
    print('\n===== 3316 header =====')
    header_size = 0
    # Display the 3316 Header
    show_hex16(fileContent, word_offset, 'channel and format')
    show_hex16(fileContent, word_offset + 1, 'timestamp 1')
    show_hex16(fileContent, word_offset + 2, 'timestamp 2')
    show_hex16(fileContent, word_offset + 3, 'timestamp 3')
    show_hex16(fileContent, word_offset + 4, 'peak high')
    show_hex16(fileContent, word_offset + 5, 'index of peak high')
    show_hex16(fileContent, word_offset + 6, 'information (upper 8 bits)')
    show_hex16(fileContent, word_offset + 7, 'accumulator gate 1')
    show_hex32(fileContent, word_offset + 8, 'accumulator gate 2')
    show_hex32(fileContent, word_offset + 10, 'accumulator gate 3')
    show_hex32(fileContent, word_offset + 12, 'accumulator gate 4')
    show_hex32(fileContent, word_offset + 14, 'accumulator gate 5')
    show_hex32(fileContent, word_offset + 16, 'accumulator gate 6')
    show_hex32(fileContent, word_offset + 18, 'MAW max')
    show_hex32(fileContent, word_offset + 20, 'MAW before trigger')
    show_hex32(fileContent, word_offset + 22, 'MAW after trigger')
    header_size += 24

    file_size_double_word = get_32_bit_number(fileContent, word_offset+header_size)
    show_hex32(fileContent, word_offset, 'Misc stuff - including sample count')
    size_of_block = file_size_double_word & 0x03FFFFFF
    print('Number of double words: 0x%x, %d' % (size_of_block, size_of_block))
    flag = (file_size_double_word & 0xF0000000) >> 28
    print('Flag: 0x%x' % (flag))
    header_size += 2

    show_hex16(fileContent, word_offset+header_size + 1, 'First data word')
    show_hex16(fileContent, word_offset+header_size, 'Second data word')
    show_hex16(fileContent, word_offset+header_size + 3, 'Third data word')
    show_hex16(fileContent, word_offset+header_size + 2, 'Fourth data word')
    print('Next offset: 0x%x, %d' % (word_offset+header_size, word_offset+header_size))
    print('\n===== End of 3316 header =====')

    return header_size, size_of_block


def show_offset_or_size(label, offset_or_size):
    print(label + '0x%x (words), 0x%x (bytes), %d (decimal words)' % (offset_or_size, 2 * offset_or_size, offset_or_size))

# This is never used
def find_super_header(fileContent, word_offset):
    print('\nLooking for super header:')
    dump_hex(fileContent, word_offset, 0x100)
    for i in np.arange(0, 0x100):  # 0x100 is arbitrary
        test_word = fileContent[word_offset + i]
        if test_word == 0xabba:
            break
    if i >= 0x100:
        return -1
    else:
        print('Start of a new super header')
        sliced, word_offset_ignore = make_NGM_file_slice(word_offset+i, 20)
        abba = fileContent[sliced]  # The 0xabbas (these could be variable in length?)
        vhex = np.vectorize(hex)
        print('Magic block: %s' % vhex(abba))
        dump_hex(fileContent, word_offset+i, 20)        # Assuming that there are 20 of these 0xabbas
        return i + 20       # This is the size of the super header


block_lengths = [0x02244c, 0x025f0b, 0x024b76]
super_header_count = 0

def find_NGM_header(fileContent, word_offset):    # Find NGM header
    global super_header_count
    # print('\nDebug header dump:')
    # dump_hex(fileContent, word_offset, 0x40)
    # Try the standard as a shortcut to avoid searching
    dump_hex(fileContent, word_offset, 0x20)
    test_size = get_32_bit_number(fileContent, word_offset + 18)
    test_size2 = get_32_bit_number(fileContent, word_offset + 14)
    test_size3 = get_32_bit_number(fileContent, word_offset + 30)
    super_header_size = 0
    # Testing specific offsets is fragile
    if test_size in block_lengths:  # Note that the first block is exempt from this test.  Which is probably not a good idea
        print('A size offset of 0x%x applies, header #: %d' % (18, NGM_header_count))
        size_of_NGM_header = show_NGM_header(fileContent, word_offset, NGM_header_count)
    elif test_size2 in block_lengths:  # Note that the first block is exempt from this test.  Which is probably not a good idea
        # The actual NGM header skips the phdr ID (see "short=True")
        print('\nDefault size offset of 0x%x applies, header #: %d' % (14, NGM_header_count))
        size_of_NGM_header = show_NGM_header(fileContent, word_offset, NGM_header_count, short=True)
    elif test_size3 in block_lengths:  # Note that the first block is exempt from this test.  Which is probably not a good idea
        # The actual NGM header skips the phdr ID (see "short=True")
        print('\nA size offset of 0x%x applies, header #: %d' % (30, NGM_header_count))
        size_of_NGM_header = show_NGM_header(fileContent, word_offset, NGM_header_count, long=True)
    else:
        print('\n============ Appears to be a non standard NGM header block #%d =============\n' % (NGM_header_count))
        for length_offset in np.arange(0, 0x100):  # 0x100 is arbitrary
            NGM_block_size = get_32_bit_number(fileContent, word_offset + length_offset)
            test_lower_length_word = fileContent[word_offset + length_offset]
            tester = 0xabba     # Some bizarre error caused this
            if test_lower_length_word == tester:
                print('!!!! ===== Super header #%d found =====' % super_header_count)
                print("Super header count / offset: %d\t0x%x" % (super_header_count, word_offset))
                super_header_count += 1
                super_header_size = length_offset + 20
                word_offset += super_header_size
                length_offset = 18  # This is the presumed length offset after a super header.  TODO: couldn't it be 14 too?
                presumed_length = get_32_bit_number(fileContent, word_offset + length_offset)
                print('Presumed length: 0x%x' % presumed_length)
                break
            elif NGM_block_size in block_lengths:
                print('Lower length word found at offset 0x%x, header #: %d' % (length_offset, NGM_header_count))
                break
        if length_offset >= 0x0ff:
            print('===>>> uh oh - Did not find the length word')  # Will blow up
            dump_hex(fileContent, word_offset, 0x100)
            return -1
        else:
            # TODO: what about headers that are 14 long?
            if length_offset == 18:
                size_of_NGM_header = show_NGM_header(fileContent, word_offset, NGM_header_count)
            elif length_offset > 18:
                extra_NGM_header_length = length_offset-18  #### TODO: I don't know about this magic 18??
                print('!!!! Extra NGM header bytes: %d' % extra_NGM_header_length)
                #### dump_hex(fileContent, word_offset, 0x100)
                size_of_NGM_header = show_NGM_header(fileContent, word_offset+extra_NGM_header_length, NGM_header_count)
                # This line implies that the extra bytes are at the beginning of the header
                size_of_NGM_header += extra_NGM_header_length     # 18 is the default length offset covered above
            else:
                print('===>>> uh-oh - unexpected length offset: %d *** Bailing out' % length_offset)        # Will blow up
                dump_hex(fileContent, word_offset, 0x100)
                exit(0)
        NGM_block_size = get_32_bit_number(fileContent, word_offset + length_offset)
        print('===>>> Length try search: 0x%x' % NGM_block_size)
        NGM_block_count = (NGM_block_size / 5013)
        print('!!!! Is this a whole number of 5013s? %0.2f' % NGM_block_count)

        if NGM_block_size not in block_lengths:
            print('===>>> uh-oh -- no length offset found.  Header size: 0x%x' % size_of_NGM_header)  # Will blow up
            return -1
    # Need to be incrementing word_offset for non-standard length - are we already?
    dump_hex(fileContent, word_offset, size_of_NGM_header)      # Dump the header
    return size_of_NGM_header + super_header_size

DEBUG = False

if __name__ == '__main__':
    parser = argparse.ArgumentParser('Parse NGM Struck File')
    parser.add_argument('file', help='NGM Struck file')
    parser.add_argument("--magic_strings", "-m", action="store_true", help="search for magic 0xabba strings onlt")
    _args = parser.parse_args()

    with open(_args.file, mode='rb') as file:  # b is important -> binary
        # fileContent = file.read()                              # Read as bytes
        # Should be able to read incrementally
        fileContent = np.fromfile(file, dtype=np.uint16)  # Read as int16s

    if _args.magic_strings:
        # This code just searches the file for 0xabba and decodes what it can
        file_offset = 0
        last_abba_offset = 0
        super_header_count = 0
        presumed_next_offset = 0
        while file_offset < len(fileContent):
            # print('searching at file offset: 0x%x' % file_offset)
            # dump_hex(fileContent, file_offset, 0x20)
            if fileContent[file_offset] == 0xabba:
                print('Predicted offset: 0x%x' % presumed_next_offset)
                print("Super header count / offset / delta since last offset / delta from prediction: %d\t0x%x\t0x%x\t0x%x"
                      % (super_header_count, file_offset, (file_offset - last_abba_offset), (file_offset - presumed_next_offset)))
                last_abba_offset = file_offset
                super_header_count += 1
                sliced, word_offset = make_NGM_file_slice(file_offset, 20)
                abba = fileContent[sliced]  # The 0xabbas (these could be variable in length?)
                vhex = np.vectorize(hex)
                print('Magic block: %s' % vhex(abba))

                file_offset += 20  # Jumps the 0xABBAs
                while True:
                    print('======= NGM header blocks ========')
                    print('\nNGM header:')
                    dump_hex(fileContent, file_offset, 0x20)        # This is the NGM header
                    print('========== Magic number indicating size of NGM header: 0x%x' % fileContent[file_offset + 1])
                    if fileContent[file_offset + 1] == 0:     # Purely empirical - and ugly
                        size_offset = 18
                    elif fileContent[file_offset + 1] == 0x10:  # Purely empirical - and ugly
                        size_offset = 30
                    elif fileContent[file_offset + 1] >= 0x30:
                        size_offset = 14
                    else:
                        print('Undecoded magic number: 0x%x' % fileContent[file_offset + 1])
                        exit(-1)

                    if DEBUG:
                        # Just pulling random numbers out of the NGM header - they escalate so they must be some sort of counter
                        print('NGM counter: 0x%x' % fileContent[file_offset+8])
                        print('NGM counter #2: 0x%x' % fileContent[file_offset+12])

                    if DEBUG:
                        print('Current offset: 0x%x' % file_offset)
                    NGM_block_size = get_32_bit_number(fileContent, file_offset + size_offset)  # This is the entire length of the block in 32-bit increments
                    print('NGM block size 0x%x' % NGM_block_size)
                    if NGM_block_size == 0:         # These are NGM headers with extra bytes at the beginning of them
                        dump_hex(fileContent, file_offset-0x40, 0x200)  # This is the NGM header

                    NGM_block_count = (NGM_block_size / 5013)
                    print('Is this a whole number of 5013s? %0.2f' % NGM_block_count)

                    # Assuming that NGM_block_count is a whole number
                    if DEBUG:
                        dump_hex(fileContent, file_offset, 0x50)
                    NGM_block_count_integer = int(NGM_block_count)
                    file_offset += size_offset+2
                    for i in range(0, NGM_block_count_integer):
                        if DEBUG:
                            dump_hex(fileContent, file_offset, 0x10)        # Dump the beginning of the of the Struct block
                        print('0x%x ' % fileContent[file_offset], end='')
                        file_offset += (2 * 5013)
                    print()
                    if DEBUG:
                        print('\n=======Next NGM header:')
                        dump_hex(fileContent, file_offset-0x100, 0x140)             # Looking for 0xABBA - but doesn't work - need to know how many NGMs there are in an 0xABBA

                # This is a bad test
                channel_and_format = fileContent[file_offset]
                if (channel_and_format & 0xF) != 0x05:  # This is not a good/reliable detection mechanism - need to use a length
                    break

                print('Super header: Guess at next jump: 0x%x' % (int(NGM_block_count) * (NGM_block_size * 2)))
                file_offset += int(NGM_block_count) * (NGM_block_size * 2)      # Need to add the NGM header size to this
                presumed_next_offset = file_offset
                #### print('Presumed next offset: 0x%x' % presumed_next_offset)
            else:
                file_offset += 1



    else:
        word_offset = show_super_header(fileContent, 0)

        # print('\n' + 40 * '#')
        # show_offset_or_size('Super header size:\t', 0xdc)
        # show_offset_or_size('NGM header size:\t', size_of_NGM_header)
        # show_offset_or_size('Struck header size:\t', 26)
        # show_offset_or_size('Data size:\t\t\t', size)
        # print(40 * '#' + '\n')


        NGM_header_count = 1
        #### NGM_this_block_header_count = 0     # this number is immediately overridden below
        S3316_data_block_counter = 1

        while word_offset < len(fileContent):
            channel_and_format = fileContent[word_offset]
            if (channel_and_format & 0xF) == 0x05:          # This is not a good/reliable detection mechanism - need to use a length
                # Process the 3316 header
                #### Struck header is 26 words long
                #### print('===== 3316 data block #%d =====' % S3316_data_block_counter)
                if S3316_data_block_counter == 1:        # False is meant to suppress the prints for diagnostic, but the offsets wont work in that case
                    header_length, size = show_3316_header(fileContent, word_offset)    # Only show the first header
                    word_offset += header_length
                else:
                    word_offset += 24

                    #### This is the size of the actual data
                    file_size_double_word = get_32_bit_number(fileContent, word_offset)     # Duplicative with above
                    size = file_size_double_word & 0x03FFFFFF       # Same as file size computed by show_3316_header
                    word_offset += 2
                word_offset += (2 * size)
                print('Struck data block #%d' % S3316_data_block_counter)
                S3316_data_block_counter += 1
            else:
                size_of_NGM_header = find_NGM_header(fileContent, word_offset)      # probably don't need all of these return values
                if size_of_NGM_header != -1:
                    print('!!!!============================= size_of_NGM_header #%d: 0x%x =======================' % (NGM_header_count, size_of_NGM_header))
                    word_offset += size_of_NGM_header
                    NGM_header_count += 1
                else:
                    # This path is never taken - find_NGM_header() handles it
                    size_of_superheader = find_super_header(fileContent, word_offset)
                    if size_of_superheader != -1:
                        word_offset += size_of_superheader
                        NGM_block_size = get_32_bit_number(fileContent, word_offset + 18)
                        print('Total size of all 3316 block under this header: 0x%x' % NGM_block_size)
                    else:
                        print('################# We are stuck looking for a new super header #################')
                        exit(0)

            # if False:
            #     i = 1
            # elif False:
            #     print('\n' + 20 * '!')
            #     print(
            #         '===>>> Channel and format don''t match 3316 header 0x05: 0x%x.  Must be a new NGM block' % channel_and_format)  # ick
            #     print('Number of 3316 blocks in previous NGM block: %d' % NGM_this_block_header_count)
            #     NGM_this_block_header_count = 1
            #     new_word_offset = find_NGM_header(fileContent, word_offset)
            #     if new_word_offset == -1:
            #         print('boo')
            #
            # # else:
            # #     NGM_this_block_header_count += 1
            #
            #
            # if False:
            #
            #     # 0x000e says no average samples
            #     # next += 1
            #     # show_hex16(fileContent, next, 'Number of average samples')
            #     # next += 1
            #     # show_hex16(fileContent, next, 'Misc 2')
            #
            #
            #     initial_word_index = word_offset
            #     word_index = word_offset
            #     found_count = 0
            #     while found_count < 30:
            #         if (fileContent[word_index] & 0xFF00) != 0x5400:
            #             print('Offset, Data word: 0x%x: 0x%x' % (word_index, fileContent[word_index]))
            #             print('Relative offset (in words): 0x%x %d' % (word_index-initial_word_index, word_index-initial_word_index))
            #             found_count += 1
            #         word_index += 1


        print('Next offset: 0x%x' % word_offset)
        print('\n==========\n')


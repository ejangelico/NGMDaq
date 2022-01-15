import argparse
import numpy as np

# Things we need to know
# TODO: decode some channel number?
# How many NGM blocks before super header?

# "Super headers" are the top level header 0xdc words long (at least the first one), and ends in the sequence 0xabba (x 20 repetitions)
#       TODO: I think the length's of these are variable - no they are not - after the first one
#           Need to print that out
#       TODO We dont know what the fields are in this header
# "Headers" are the NGM headers
# "3316 headers" are the Struck headers
#   We currently only display the decode for the first of these
#       TODO: These are variable length and we have not figured out how a deterministic way to determine the length (see find_NGM_header_length())
# TODO: what are the struck headers called?

# Note: file is read as int16's so offsets in code are in 16 bit steps
#   TODO: But the hexdump output indices are in bytes - so we are doing conversions all over here
#           All of the 'next's are offsets in bytes
#           Is this already resolved?

# The decode process is as follows:
#   Assume that the initial header is 0xdc words (16-bit) long and ends in 20 0xabba words
#   While true:
#       Decode an NGM header (which are variable length) by search for a 32-bit length word out of
#           a set of empirically determined length words (this is very fragile).
#           This process determines the length of the NGM header block
#       Assume the 32-bit length quantity found above is the total number of 32-bit words
#           in all of the Struck blocks represented by this NGM header
#       Assume all Struck blocks are a 13 32-bit word header followed by 5000 32-bit data words
#           (The 13 32-word headers are in the Struck documentation)
#       Determine the number of Struck data blocks by dividing the length in the NGM header by 5013
#       Iterate over the number of Struck data blocks.  (Currently not decoding each header)
# All super headers are 0x24 words long except the first one

# Offset in words and length in 16 bit? quantities (or is that really 8 = bytes) is confusing
def make_NGM_file_slice(offset_in_words, length_in_int16s):
    return slice(offset_in_words, (offset_in_words + length_in_int16s)), offset_in_words + length_in_int16s

def show_hex16(file, offset, name):
    print(name+': 0x%x' % file[offset])

def show_hex32(file, offset, name):
    bits_32 = get_32_bit_number(file, offset)
    print(name+': 0x%x' % bits_32)

# Not always a file
def get_32_bit_number(file, offset):
    high_order_16 = file[(offset + 1)] << 16
    return (high_order_16 + file[offset])

def dump_hex(file_data, offset, count, label=''):
    print('\n==== DUMP %s | offset 0x%x' % (label, offset))
    print('0x%05x: ' % 0, end='')
    for i in np.arange(offset, offset+count):
        print('0x%04x ' % file_data[i], end='')
        if (i - offset + 1) % 16 == 0:
            print()
            print('0x%05x: ' % (i - offset + 1), end='')
    print()

# This method is currently only used to display the first super header
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

def show_NGM_header(file_data, word_offset, NGM_header_count, NGM_header_start_offset=0, short=False):  # NGM_header_count may be unused
    #### print('\n===== NGM header #%d =====' % NGM_header_count)
    vhex = np.vectorize(hex)
    offset_into_header = word_offset + NGM_header_start_offset
    if not short:
        phdrid_slice, word_offset_ignore = make_NGM_file_slice(offset_into_header, 4)
        phdrid = file_data[phdrid_slice]
        print('phdrid: %s' % vhex(phdrid))
        offset_into_header += 4

    hdrid_slice, word_offset_ignore = make_NGM_file_slice(offset_into_header, 2)
    hdrid = file_data[hdrid_slice]
    print('hdrid: %s' % vhex(hdrid))
    offset_into_header += 2

    sliced, word_offset_ignore = make_NGM_file_slice(offset_into_header, 12)
    triggerstatspill = file_data[sliced]
    print('triggerstatspill: %s' % vhex(triggerstatspill))

    number_of_struct_data_blocks_under_this_NGM_header = get_32_bit_number(triggerstatspill, 2) # this is kind of a hack.  this number appears twice in the triggerstatspill
    offset_into_header += 12

    # This is the length of this block of reads before the next NGM header (in words) = 0x2244c = decimal 140364
    # For 0x2244c there are 28 Struck 3316 data blocks in one NGM block
    total_struck_blocks_size = get_32_bit_number(file_data, offset_into_header)
    print('databuffferread: 0x%x' % total_struck_blocks_size)
    offset_into_header += 2     # Not necessary - but keeps the accounting correct
    #### print('===== End of NGM header #%d =====\n' % NGM_header_count)
    return number_of_struct_data_blocks_under_this_NGM_header, total_struck_blocks_size       # This should really not be returned from here

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

    # Duplicative with get_size_of_struck_block_data
    size_of_data_in_block = file_size_double_word & 0x03FFFFFF
    print('Number of double words: 0x%x, %d' % (size_of_data_in_block, size_of_data_in_block))
    flag = (file_size_double_word & 0xF0000000) >> 28
    print('Flag: 0x%x' % (flag))
    header_size += 2

    show_hex16(fileContent, word_offset+header_size + 1, 'First data word')
    show_hex16(fileContent, word_offset+header_size, 'Second data word')
    show_hex16(fileContent, word_offset+header_size + 3, 'Third data word')
    show_hex16(fileContent, word_offset+header_size + 2, 'Fourth data word')
    print('Next offset: 0x%x, %d' % (word_offset+header_size, word_offset+header_size))
    print('===== End of 3316 header =====\n')

def get_size_of_struck_block_data(fileContent, word_offset):
    file_size_double_word = get_32_bit_number(fileContent, word_offset+24)
    size_of_data_in_block = file_size_double_word & 0x03FFFFFF
    # print('Number of double words in struck block: 0x%x, %d' % (size_of_data_in_block, size_of_data_in_block))
    return size_of_data_in_block

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


def decode_super_header(fileContent, word_offset, pre_abba_length_offset):
    global super_header_count
    # Found a super header
    print('\n\n===== Super header #%d  =====' % super_header_count)
    super_header_count += 1
    super_header_size = pre_abba_length_offset + 20
    print("Super header size: 0x%x" % (super_header_size))
    dump_hex(fileContent, word_offset, super_header_size, label='super header')
    #length_offset = 18  # This is the presumed length offset after a super header.  TODO: couldn't it be 14 too?
    #presumed_length = get_32_bit_number(fileContent, word_offset + length_offset)
    #print('Presumed length: 0x%x' % presumed_length)  #### TODO: does this "presumed length" have any meaning?
    print(30 * '=' + '\n')
    return super_header_size


def decode_struct_blocks(fileContent, word_offset, number_of_struct_blocks, first_block=False):
    S3316_data_block_relative_counter = 1
    total_data_size = 0
    for block_number in range(0, number_of_struct_blocks):
        # Decode Struck blocks in sequence
        channel_and_format = fileContent[word_offset]
        if (channel_and_format & 0xF) != 0x05:  # This is not a good/reliable detection mechanism - need to use a length
            print('======== Unexpected Struck block channel_and_format: 0x%x' % (channel_and_format & 0x0F))
            # Do nothing more?

        # Process the Struck 3316 header
        #### Struck header is 13 32-words long  TODO: is it always??
        if first_block:
            show_3316_header(fileContent, word_offset)  # Only show the first header
            first_block = False
        #### This is the size of the actual data
        # Will they be the same for every block in this NGM group
        size_of_struck_block_data = get_size_of_struck_block_data(fileContent, word_offset)
        word_offset += 26  # Fixed size?
        word_offset += (2 * size_of_struck_block_data)
        total_data_size += 26 + (2 * size_of_struck_block_data)
        if S3316_data_block_relative_counter == 1:
            print('Struck data blocks: ', end='')
        print(' #%d ' % S3316_data_block_relative_counter, end='')  # Need a return at the end of this...
        S3316_data_block_relative_counter += 1
    print('\n')
    return size_of_struck_block_data, total_data_size       # Assumes that the size_of_struck_block_data is the same for all blocks

block_lengths = [0x02244c, 0x025f0b, 0x024b76]
super_header_count = 1

# Find the length of the NGM header using empirical lengths of the NGM header and the Struck data blocks that we figured out
# Empirically, the sizes of the NGM header can be 16, 20, 32, 0x64, 0x44, 0x24 (those are all of them in our test file

def find_NGM_header_length(fileContent, word_offset):    # Find NGM header

    # print('\nDebug header dump:')
    # dump_hex(fileContent, word_offset, 0x40)
    # Try the standard as a shortcut to avoid searching
    # print('\n===== NGM header #%d =====' % NGM_header_count)
    # test_size = get_32_bit_number(fileContent, word_offset + 18)
    # test_size2 = get_32_bit_number(fileContent, word_offset + 14)
    # test_size3 = get_32_bit_number(fileContent, word_offset + 30)
    # test_size4 = get_32_bit_number(fileContent, word_offset + 0x62)
    # test_size5 = get_32_bit_number(fileContent, word_offset + 0x42)
    # test_size6 = get_32_bit_number(fileContent, word_offset + 0x22)
    # super_header_size = 0
    # # Testing specific offsets is fragile
    # if test_size in block_lengths:  # Note that the first block is exempt from this test.  Which is probably not a good idea
    #     print('A size offset of 0x%x applies, NGM header #%d' % (18, NGM_header_count))
    #     size_of_NGM_header = show_NGM_header(fileContent, word_offset, NGM_header_count)
    # elif test_size2 in block_lengths:  # Note that the first block is exempt from this test.  Which is probably not a good idea
    #     # The actual NGM header skips the phdr ID (see "short=True")
    #     print('\nDefault size offset of 0x%x applies, NGM header #%d' % (14, NGM_header_count))
    #     size_of_NGM_header = show_NGM_header(fileContent, word_offset, NGM_header_count, short=True)
    # elif test_size3 in block_lengths:  # Note that the first block is exempt from this test.  Which is probably not a good idea
    #     print('\nA size offset of 0x%x applies, NGM header #%d' % (30, NGM_header_count))
    #     size_of_NGM_header = show_NGM_header(fileContent, word_offset, NGM_header_count)
    # elif test_size4 in block_lengths:  # Note that the first block is exempt from this test.  Which is probably not a good idea
    #     print('\nA size offset of 0x%x applies, NGM header #%d' % (0x62, NGM_header_count))
    #     size_of_NGM_header = show_NGM_header(fileContent, word_offset, NGM_header_count) # TODO: This will probably be all wrong
    # elif test_size5 in block_lengths:  # Note that the first block is exempt from this test.  Which is probably not a good idea
    #     print('\nA size offset of 0x%x applies, NGM header #%d' % (0x42, NGM_header_count))
    #     size_of_NGM_header = show_NGM_header(fileContent, word_offset, NGM_header_count)  # TODO: This will probably be all wrong
    # elif test_size6 in block_lengths:  # Note that the first block is exempt from this test.  Which is probably not a good idea
    #     print('\nA size offset of 0x%x applies, NGM header #%d' % (0x22, NGM_header_count))
    #     size_of_NGM_header = show_NGM_header(fileContent, word_offset, NGM_header_count)  # TODO: This will probably be all wrong

    if False:
        i = 1
    else:
        NGM_header_size = 0
        super_header_size = 0
        number_of_data_blocks = 0
        NGM_block_count = 0

        # A brute force search is ugly
        # TODO: skip by 2 in the loop

        total_struck_blocks_size = 0
        for length_offset in np.arange(0, 0x100):  # 0x100 is arbitrary search length       # Is it enough?
            struck_block_size_in_NGM_header = get_32_bit_number(fileContent, word_offset + length_offset)
            test_block_length = get_32_bit_number(fileContent, word_offset + length_offset)
            tester = 0xabbaabba
            # print('Lower length word this try: 0x%x' % test_block_length)
            if test_block_length in block_lengths:      # TODO: This is super fragile
                print('\n===== NGM header #%d =====' % NGM_header_count)
                print('Lower length word found at offset 0x%x, header #: %d' % (length_offset, NGM_header_count))
                print('>>> Struck block size: %d, 0x%x' % (struck_block_size_in_NGM_header, struck_block_size_in_NGM_header))
                NGM_header_size = length_offset+2
                print('--->>> NGM header size: %d, 0x%x' % (NGM_header_size, NGM_header_size))
                NGM_block_count = (test_block_length / 5013)       # TODO: 5013 cannot be hardwired
                print('Is this a whole number of 5013s? %0.2f' % NGM_block_count)
                NGM_block_count = int(NGM_block_count)

                short = False
                if NGM_header_size == 16:
                    short = True
                    NGM_header_start_offset = 0
                else:
                    NGM_header_start_offset = NGM_header_size - 20
                # TODO: I don't like returning things from a "show" method
                number_of_data_blocks, total_struck_blocks_size = show_NGM_header(fileContent, word_offset,
                                                                                  NGM_header_count,
                                                                                  NGM_header_start_offset=NGM_header_start_offset,
                                                                                  short=short)
                # Need to be incrementing word_offset for non-standard length - are we already?
                dump_hex(fileContent, word_offset, NGM_header_size, 'NGM header')  # Dump the header
                print('Internal NGM block count: %d' % fileContent[word_offset+1])
                if NGM_block_count != number_of_data_blocks:         # Need to give better names to these
                    print('========!!!!!!!!! Uh oh.  mismatch between Struck block counts.')
                    print('\tCount by dividing by 5013: %d vs. direct count from spill header: %d' % (NGM_block_count, number_of_data_blocks))
                    print('\tUsing division count.  Which is very fragile.')
                print('==== size_of_NGM_header #%d: 0x%x ====' % (NGM_header_count, NGM_header_size))
                print('===== End of NGM header #%d =====\n' % NGM_header_count)

                break
            elif test_block_length == tester:       # It's a super block
                super_header_size = decode_super_header(fileContent, word_offset, length_offset)
                break
        if length_offset >= 0x0ff:
            print('===>>> uh oh - Did not find the NGM length word or a 0xabba super header')  # Will blow up
            dump_hex(fileContent, word_offset, 0x100)
            exit(0)
    return NGM_header_size, super_header_size, NGM_block_count, total_struck_blocks_size

DEBUG = False

if __name__ == '__main__':

    parser = argparse.ArgumentParser('Parse NGM Struck File')
    parser.add_argument('file', help='NGM Struck file')
    parser.add_argument("--magic_strings", "-m", action="store_true", help="search for magic 0xabba strings only")
    _args = parser.parse_args()

    with open(_args.file, mode='rb') as file:  # b is important -> binary
        # fileContent = file.read()                              # Read as bytes
        # Should be able to read incrementally
        fileContent = np.fromfile(file, dtype=np.uint16)  # Read as int16s

    # This code just searches the file for 0xabba and decodes what it can
    if _args.magic_strings:
        file_offset = 0
        last_abba_offset = 0
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

                    # This is where we run through the Struct blocks
                    for i in range(0, NGM_block_count_integer):
                        if DEBUG:
                            dump_hex(fileContent, file_offset, 0x10)        # Dump the beginning of the of the Struck block
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

    # Decode the file with what we know (so far) about the encoding
    else:
        try:
            # Process the first super header (including the extra bytes at the beginning of the file
            print('===== Super header #%d =====' % super_header_count)
            super_header_count += 1
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

            first_struck_block = True
            while word_offset < len(fileContent):
                # All of these return values are a mess
                size_of_NGM_header, size_of_super_header, number_of_struck_data_blocks, total_struck_blocks_data_size = \
                    find_NGM_header_length(fileContent, word_offset)

                # Indicator that we found an NGM header
                if size_of_NGM_header != 0:
                    word_offset += size_of_NGM_header
                    NGM_header_count += 1
                    size_of_struck_block_data, total_data_size = decode_struct_blocks(fileContent, word_offset, number_of_struck_data_blocks, first_block=first_struck_block)
                    first_struck_block = False
                    word_offset += total_data_size
                    S3316_data_block_counter += number_of_struck_data_blocks

                # Indicator that we found a super header
                elif size_of_super_header != 0:
                    word_offset += size_of_super_header
                    super_header_count += 1
                else:
                    print('No size coming from find_NGM_header_length() - bailing out')
                    exit(-1)

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


            print('\n Next offset: 0x%x' % word_offset)
            print('\n==========\n')
        except IndexError as ie:
            print('Exception raised reading file.  Presumably we ran off the end of it.  Current offset: %d' % word_offset)
            dump_hex(fileContent, word_offset, 0x20)
            print()
            print('Total Struck data blocks: %d' % (S3316_data_block_counter - 1))
            print('Total NGM blocks: %d' % (NGM_header_count - 1))
            print('Total super blocks: %d' % (super_header_count - 1))
            exit(0)

        print('\nTotal Struck data blocks: %d' % (S3316_data_block_counter - 1))
        print('Total NGM blocks: %d' % (NGM_header_count - 1))
        print('Total super blocks: %d' % (super_header_count - 1))


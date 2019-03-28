'''
Brian Hoole
ICS 423
Lab 3
'''

import io
import binascii

# We are settting up the tables here to make the DES function a bit cleaner to read
IP = [58, 50, 42, 34, 26, 18, 10, 2,
      60, 52, 44, 36, 28, 20, 12, 4, 
      62, 54, 46, 38, 30, 22, 14, 6,
      64, 56, 48, 40, 32, 24, 16, 8, 
      57, 49, 41, 33, 25, 17, 9, 1, 
      59, 51, 43, 35, 27, 19, 11, 3, 
      61, 53, 45, 37, 29, 21, 13, 5, 
      63, 55, 47, 39, 31, 23, 15, 7
     ]
    
IP_inverse = [40, 8, 48, 16, 56, 24, 64, 32, 
              39, 7, 47, 15, 55, 23, 63, 31, 
              38, 6, 46, 14, 54, 22, 62, 30,
              37, 5, 45, 13, 53, 21, 61, 29,
              36, 4, 44, 12, 52, 20, 60, 28,
              35, 3, 43, 11, 51, 19, 59, 27,
              34, 2, 42, 10, 50, 18, 58, 26,
              33, 1, 41, 9, 49, 17, 57, 25
             ]
    
expansion_bit = [32, 1, 2, 3, 4, 5,
                 4, 5, 6, 7, 8, 9,
                 8, 9, 10, 11, 12, 13,
                 12, 13, 14, 15, 16, 17,
                 16, 17, 18, 19, 20, 21,
                 20, 21, 22, 23, 24, 25,
                 24, 25, 26, 27, 28, 29,
                 28, 29, 30, 31, 32, 1
                ]
    
straight_p_box = [16, 7, 20, 21,
                  29, 12, 28, 17,
                  1, 15, 23, 26,
                  5, 18, 31, 10,
                  2, 8, 24, 14,
                  32, 27, 3, 9,
                  19, 13, 30, 6,
                  22, 11, 4, 25
                 ]

s1 = [14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7,
      0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8,
      4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0,
      15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13
     ]

s2 = [15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10,
      3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5, 
      0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15,
      13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9
     ]

s3 = [10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8,
      13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1,
      13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7,
      1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12
     ]

s4 = [7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15,
      13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9,
      10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4,
      3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14
     ]

s5 = [2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9, 
      14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6, 
      4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14, 
      11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3
     ]

s6 = [12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11,
      10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8,
      9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6,
      4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13
     ]

s7 = [4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1,
      13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6,
      1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2,
      6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12
     ]

s8 = [13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7,
      1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2,
      7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8,
      2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11
     ]

s_boxes = [s1, s2, s3, s4, s5, s6, s7 ,s8]

p = [16, 7, 20, 21,
     29, 12, 28, 17,
     1, 15, 23, 26,
     5, 18, 31, 10,
     2, 8, 24, 14,
     32, 27, 3, 9,
     19, 13, 30, 6,
     22, 11, 4, 25
    ]

pc1 = [57, 49, 41, 33, 25, 17, 9,
       1, 58, 50, 42, 34, 26, 18,
       10, 2, 59, 51, 43, 35, 27,
       19, 11, 3, 60, 52, 44, 36,
       63, 55, 47, 39, 31, 23, 15,
       7, 62, 54, 46, 38, 30, 22,
       14, 6, 61, 53, 45, 37, 29,
       21, 13, 5, 28, 20, 12, 4
      ]

pc2 = [14, 17, 11, 24, 1, 5, 
       3, 28, 15, 6, 21, 10,
       23, 19, 12, 4, 26, 8,
       16, 7, 27, 20, 13, 2,
       41, 52, 31, 37, 47, 55,
       30, 40, 51, 45, 33, 48,
       44, 49, 39, 56, 34, 53,
       46, 42, 50, 36, 29, 32
      ]

iteration_shift = {0:1, 1:1, 2:2, 3:2, 4:2, 5:2, 6:2, 7:2,
                   8:1, 9:2, 10:2, 11:2, 12:2, 13:2, 14:2, 15:1
                  }

iteration_shift_1 = {1:1, 2:1, 3:2, 4:2, 5:2, 6:2, 7:2, 8:2,
                   9:1, 10:2, 11:2, 12:2, 13:2, 14:2, 15:2, 15:1
                  }
# # # # #  Utility Functions # # # # #
"""
Function: xor
Formatting string based from https://stackoverflow.com/questions/38204582/how-to-xor-two-binary-strings-in-python

Input
a - binary string to xor
b - binary string to xor

Output
result - binary string that is the result of the a xor b
"""
def xor(x, y):
    xor_result = int(x,2) ^ int(y, 2)
    result = '{0:0{1}b}'.format(xor_result, len(x))
    return result


"""
Function: rotate_left

Input
bin_string - binary string that is to be rotated left
rotations - number of rotations

Output
result - binary string that is the result of the rotation
"""

def rotate_left(bin_string, rotations):
    result = bin_string
    for rotate in range(rotations):
        first_bit = result[0]
        result = result[1:] + first_bit
    return result

"""
Function: key_maker

Input
key_binary - binary string

Output
keychain - list of 16 keys 
""" 

def key_maker(key_binary):
    key_permutated = ""
    keychain = ["" for x in range(16)]
    for k in range(56):
        key_permutated += key_binary[pc1[k]-1]
    # Split left and right for key
    left_key = key_permutated[:28]
    right_key = key_permutated[28:]
    for des_round in range(16):
        p_key=""
        # Left Shift (or rather left rotate) both left and right key
        left_key = rotate_left(left_key, iteration_shift[des_round])
        right_key = rotate_left(right_key, iteration_shift[des_round])
        # Combine left and right key and use pc2 to get p_key
        k_combine = left_key + right_key
        for l in range(len(pc2)):          
            p_key += k_combine[pc2[l]-1]
        keychain[des_round] = p_key
    return keychain

"""
Function: s_box_value

Input
six_bits - Six bits that determine value
s_box - S-Box that is being used

Output
result - binary string value that is selected from s_box
"""
def s_box_value(six_bits, s_box):
    row = six_bits[0] + six_bits[5]
    column = six_bits[1:5]
    row_value = int(row, 2)
    column_value = int(column, 2)
    result_value = s_box[(row_value * 16) + column_value]
    result_binary_string = ''.join('{0:04b}'.format(result_value, 'b'))
    return result_binary_string

"""
Function: binary_ascii

Input
bits - 64-bit string to be converted

output
64-bit ascii string from binary input
"""
def binary_to_ascii(bits):
    message = ""
    while bits != "":
        i = chr(int(bits[:8],2))
        message += i
        bits = bits[8:]
    return message

# # # # # DES Functions # # # # #
"""
Function: des_function

Input
binary_input - 32 length binary string (ie. "0101")
key - 48 length binary string that represents the key

Output
result - 32 length binary string
"""
def des_function(binary_input, key):
    result = ""
    s_result = ""
    expanded = ""
    # Expansion P-block
    for i in range(48):
        expanded += binary_input[expansion_bit[i]-1]
    # XOR with Key
    expanded = xor(expanded, key)
    # S-Boxes
    for s in range(len(s_boxes)):
        s_result += s_box_value(expanded[:6], s_boxes[s])
        expanded = expanded[6:]
    # Straight P-box
    for t in range(len(straight_p_box)):
        result += s_result[straight_p_box[t]-1]
    return result
   
"""
Function: des

Input
mode - integer for encrypt (1) or decrypt (0)
text - string to be encrypted or decrypted
key - key for encryption/decryption we are assuming that key is in ASCII

Output
result - string containing the result
"""
def des(mode, text, key):    
    result = ""
    final_text = ""
    text_permutated = ""
    keychain = ["" for x in range(16)]
    key_binary = ''.join('{0:08b}'.format(ord(x), 'b',) for x in key)
    if(mode == 1):
       text_binary = ''.join('{0:08b}'.format(ord(x), 'b',) for x in text)
    else: 
       text_binary = text
    # Initial Permutation for text and key: IP and pc1
    for j in range(64):
        text_permutated += text_binary[IP[j]-1]
    # Split left and right for text
    left_text = text_permutated[:32]
    right_text = text_permutated[32:]
    # Create keychain list
    keychain = key_maker(key_binary)
    # Manipulate text with keychain based on mode
    for des_round in range(16):
        if (mode == 1):
            function_result = des_function(right_text, keychain[des_round])
        elif (mode == 0):
            function_result = des_function(right_text, keychain[(15-des_round)%16])
        # XOR function_result with left_text
        left_text=xor(function_result, left_text)
        # Swap left_text and right_text unless on the last round
        if des_round < 15:
            buffer_text=left_text
            left_text=right_text
            right_text=buffer_text
        else:
            final_text = left_text + right_text
    # Final Permutation: IP_Inverse
    for r in range(len(final_text)):
        result += final_text[IP_inverse[r]-1]
    if(mode == 0):
        result = (binary_to_ascii(result))
    return result


def main():
    # DO NOT MODIFY #  
    mode = input("Encrypt (1) or Decrypt (0): ")    
    while (mode != '0' and mode != '1'):
        print("invalid entry, please try again")
        mode = input("Encrypt (1) or Decrypt (0): ")
    mode = int(mode)
    text = input("Please enter the text: ")
    key = input("Please enter the key:  ")
    while len(key) != 8:
        print("Incorrect key, please enter a key with a length that is 8.")
        key = input("Please enter the key: ")
    # IV is not going to be implemented here as we are going ot use ECB instead of other modes.
    # Pad if text is not a multiple of 8 (a char is a byte and 8 bytes is 64 bits)
    if len(text) % 8 != 0:
        text += ''.join(["0" for x in range(8 - (len(text) % 8))])
    # DO NOT MODIFY #
    # Break into blocks 
    if len(text) > 8:
        result = ""
        count = 8
        if mode == 0:
            count = 64
        for index in range(0,int(len(text)/count)):
            result += des(mode, text[index*count: index*count+count], key)
        print(result)
    else:
        print(des(mode, text, key))
    
        

if __name__ == "__main__":
    main()

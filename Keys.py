import string
import io
import os
import math
from Crypto.PublicKey import RSA
from Crypto.Random import get_random_bytes
from Crypto.Cipher import AES, PKCS1_OAEP



##### UTILITY FUNCTION #####
"""
Function: format_file

Args: 
    message_file: filename of saved message to be encoded
    
Returns:
    None - file is saved as output
"""
def format_file(message_file):
    print("Formatting message file")
    with open(message_file, 'r') as read_file:
        message = read_file.read()
    print("Deleting old file...")
    os.remove(message_file)
    print("Creating formatted file with same name as previous...")
    formated = message.encode("utf-8")
    with open(message_file, 'wb') as write_file:
        write_file.write(formated)
    return

##### UTILITY FUNCTION #####


"""
Function: dh

Args:
    p: modulus
    g: base
    s: secret 

Returns:
    Result of diffie-hellman
"""
def dh(p, b, s):
    result = math.pow(b,s)
    result = result%p
    return result


"""
Function: pki

Args:
    key_file: base key file name to be used, generated. (ie. key_file=test results in test_public.pem, test_private.pem)
    key_mode: 1 to generate 0 to read
    message_file: message file to be encrypted
    message_mode: 1 to encrypt 0 to decrypt 

Returns:
    None - encrypted/derypted file is saved/key is generated and saved
"""
def pki(key_file, key_mode, message_file, message_mode):
    file_name = message_file.split(".")[0]
    
    if key_mode == 1:
        key = RSA.generate(2048)
        key_private = key.export_key()
        key_public = key.publickey().export_key()
        # generate key
        with open(key_file + "_public.pem", "wb") as key_f:
            key_f.write(key_public)
        with open(private_key + "_private.pem", "wb") as key_f:
            key_f.write(key_private)
        print(key_public.publickey().export_key())

    if message_mode == 1:
        encrypted_message = ""
        # encrypt file with public key
        encrypted_message = (open(message_file).read()).encode("utf-8")
        file_out = open(file_name+"_encrypted.bin", "wb")
        
        recipient_key = RSA.import_key(open(key_file + "_public.pem").read())
        session_key = get_random_bytes(16)

        cipher_rsa = PKCS1_OAEP.new(recipient_key)
        enc_session_key = cipher_rsa.encrypt(session_key)

        cipher_aes = AES.new(session_key, AES.MODE_EAX)
        ciphertext, tag = cipher_aes.encrypt_and_digest(encrypted_message)
        [ file_out.write(x) for x in (enc_session_key, cipher_aes.nonce, tag, ciphertext) ]

    else:
        decrypted_message = ""
        # decrypt file with private key
        file_in = open(file_name +"_encrypted.bin", "rb")

        private_key = RSA.import_key(open(key_file + "_private.pem").read())

        enc_session_key, nonce, tag, ciphertext = \
            [ file_in.read(x) for x in (private_key.size_in_bytes(), 16, 16, -1) ]

        cipher_rsa = PKCS1_OAEP.new(private_key)
        session_key = cipher_rsa.decrypt(enc_session_key)

        cipher_aes = AES.new(session_key, AES.MODE_EAX, nonce)
        data = cipher_aes.decrypt_and_verify(ciphertext,tag)

        with open(file_name+"_decrypted.txt", "w") as decrypted_file:
            decrypted_file.write(data.decode("utf-8"))
        print(data.decode("utf-8"))

    return

def main():
    runFunction = int(input("Please select which function to run: (1) Diffie-Helman (2) PKI: "))
    # Args for dh
    if runFunction == 1:
        p = int(input("Please provide integer for modulus - p: "))
        b = int(input("Please provide integer for base - b: "))
        s = int(input("Please provide integer for secret - s: "))
        print ("Diffie Hellman secret key generated is: ", dh(p, b, s))
    # Args for pki
    else:
        key_mode = int(input("Please enter - (1) Generate key (0) Read key: "))
        message_mode = int(input("Please enter - (1) Encrypt message (0) Decrypt message: "))
        key_file = input("Provide the key file to be used/generated: ")
        message_file = input("Provide the message file to be encrypted or decrypted: ")
        if message_mode == 1:
            message_format = input("Is message formatted in UTF-8? (y/n): ")
            if message_format == "n":
                format_file(message_file)
        pki(key_file, key_mode, message_file, message_mode)
    
    

if __name__ == '__main__':
    main()
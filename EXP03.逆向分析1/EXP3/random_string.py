import random
import string
 
def random_string_generator(str_size, allowed_chars):
    return ''.join(random.choice(allowed_chars) for _ in range(str_size))

with open("xba.txt", "wb") as f:
    f.write(random_string_generator(256 + 5, string.ascii_letters).encode())
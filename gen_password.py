import hmac
import hashlib

secret = '20060119swSW'
timestamp = '2025020418'

password = hmac.new(
    secret.encode('utf-8'),
    timestamp.encode('utf-8'),
    hashlib.sha256
).hexdigest()

print('Password:', password)
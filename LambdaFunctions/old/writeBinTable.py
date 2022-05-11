import json
import boto3
import time
import base64

client=boto3.client('dynamodb')

def lambda_handler(event, context):
    
    
    base64_message = event["uplink_message"]["frm_payload"]
    base64_bytes = base64_message.encode('ascii')
    message_bytes = base64.b64decode(base64_bytes)
    message = message_bytes.decode('ascii').split('|')
    
    print(message)
    
    data = client.update_item(
        ExpressionAttributeNames={
            '#FILL': 'last_fill_level',
            '#TIME': 'last_fill_timestamp'
        },
        
        ExpressionAttributeValues={
            ':fill': {
                'N': message[1]
                },
            ':time': {
                'S': str(time.time())
            }
         },
        Key= {
            'id': {
                'S': message[0]
            }
        },
        ReturnValues='ALL_NEW',
        TableName='bin_table',
        UpdateExpression='SET #FILL = :fill, #TIME = :time'
        
        
        
    )
    
    print(data)
    
    return {
        'statusCode': 200
    }

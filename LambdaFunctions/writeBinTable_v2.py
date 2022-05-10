import json
import boto3
import time
import base64

client=boto3.client('dynamodb')

def lambda_handler(event, context):
    
    #extract the message payload and decoding it from base64
    base64_message = event["uplink_message"]["frm_payload"]
    base64_bytes = base64_message.encode('ascii')
    message_bytes = base64.b64decode(base64_bytes)
    message = message_bytes.decode('ascii')
    
    dev_eui = event['end_device_ids']['dev_eui']
    
    #checking if DevEUI is already in use
    data = client.scan(
        TableName='binTable',
    )
    
    bool = False 
    for element in data['Items']:
        if element['DevEUI']['S'] == dev_eui:
            bool = True
    
    if not bool: #if DevEUI already in use return an error
        return {
            'statusCode': 400
        }
    
    data = client.update_item( #otherwise: add the element
        ExpressionAttributeNames={
            '#FILL': 'last_fill_level',
            '#TIME': 'last_fill_timestamp'
        },
        
        ExpressionAttributeValues={
            ':fill': {
                'N': message
                },
            ':time': {
                'S': str(time.time())
            }
         },
        Key= {
            'DevEUI': {
                'S': dev_eui
            }
        },
        ReturnValues='ALL_NEW', 
        TableName='binTable',
        UpdateExpression='SET #FILL = :fill, #TIME = :time'
        
        
        
    )
    
    return {
        'statusCode': 200
    }
import json
import boto3
import time

client=boto3.client('dynamodb')

def lambda_handler(event, context):
    
    body = json.loads(event['body'])
    
    data = client.scan(
        TableName='bin_table',
    )
    
    max = 0
    
    for element in data['Items']:
        if int(element['id']['S']) > max:
            max = int(element['id']['S'])
    max += 1
    
    
    result = client.put_item(
        Item={
            'id':{
                'S': str(max)
            },
            'lat':{
                'S': body['lat']
            },
            'lng': {
                'S': body['lng']
            },
            'last_fill_level': {
                'N': '0'
            },
            'last_fill_timestamp': {
                'S': str(time.time())
            }
        },
        ReturnConsumedCapacity='TOTAL',
        TableName='bin_table',
        )
    
    return {
        'statusCode': 200,
        "headers": {
            "Content-Type": "application/json"
        },
        "body": json.dumps({
            "id": str(max)
        })
    }
    

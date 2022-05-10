import json
import boto3
import time

client=boto3.client('dynamodb')

def lambda_handler(event, context):
    #returning the whole table
    data = client.scan(
        TableName='binTable',
    )
    
    return {
        'statusCode': 200,
        'body': json.dumps(data["Items"])
    }
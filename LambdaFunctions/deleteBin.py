import json
import boto3


def lambda_handler(event, context):
    
    dynamodb = boto3.client('dynamodb')
    
    body = json.loads(event['body'])
    
    #extracting the DevEUI from the ID
    data = dynamodb.scan(
        TableName='binTable',
    )
    
    dev_eui = ""
    
    for element in data['Items']:
        if element['id']['S'] == body['id']:
           dev_eui = element['DevEUI']['S']

    if dev_eui == "": #if DevEUI not present return error
        return {
            'statusCode': 400,
        }
        

    response = dynamodb.delete_item( #otherwise: delete the element
        TableName='binTable',
        Key={
            'DevEUI': {
                'S': dev_eui
            }
        }
    )
    
    return {
        'statusCode': 200,
    }
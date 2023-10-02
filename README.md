# gmail_web_client_cpp
A simple GMail web client

# Server setup

## Step 1: install dependencies
Using vcpkg install drogon web framework and its dependencies

## Step 2: Create a new google project 
Go to https://console.cloud.google.com/ and create a new project.

Add the Gmail API to the project.

register redirect uri to {yourhostname}/redirect in the project.

Create oauth credentials for the project.

## Step 3: Configure the server

- Create config.json using drogon documentation.
- export CLIENT_ID environment variable with the client id from the oauth credentials.
- export CLIENT_SECRET environment variable with the client secret from the oauth credentials.
- export REDIRECT_URI environment variable with the redirect uri from the oauth credentials.

## Step 4: Build and run the server

Run the server and enjoy the app!
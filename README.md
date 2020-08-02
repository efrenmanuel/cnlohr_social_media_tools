# various social media tools

## Some programs you'll need to install.

```
sudo apt-get install libssl1.0-dev
```

## Youtube API stuff

### OAUTH

First, you will need your youtube oauth creds. 

Log into your API Dashboard.
 * https://console.developers.google.com/
 * Select your project and go to the youtube v3 api section.
 * Create new oauth credentials (or use existing) 
 * Select type for new oauth app to be "other" or "Desktop app"
 * Get the CLIENT_ID and note the SECRET_KEY.
 * Note: Client ID should be something like: ############-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.apps.googleusercontent.com
 * Note: Secret key looks like: #_XXXXXXXXXXXXXXXXX
 * Write them into `.client_id.txt` and `.client_secret.txt` to the main `cnlohr_social_media_tools` folder.

Next, get your oauth keys.
```
cd ytoauthhelper
./ytoauthhelper
```

Follow on-screen prompts.  It should write your oauth key into `.oauthtoken.txt`.

### Key

Go to the dashboard (see above) Make new API key.  Put it in `.ytapikey.txt`.

## Doing streamstats

Find your video video id by looking on the livechat popout, i.e. 16NSQORHRqU

For example, you can do the following:

```
cd ytstreamstats
./ytstreamstats [stream id]
```

or

```
./runytchatmon.sh EiEKGFVDRzd5SVd0VndjRU5nX1pTLW5haGc1ZxIFL2xpdmU | tabformatter/tabformatter | tee chatlog.txt |  ./addtime.sh | ./rundiscordposter.sh
```

or

```
cd ytposter
./ytposter [stream id] chat message
```


Once these tools are running, you can use streambuddy.

```
cd streambuddy
./streambuddy
```

## ColorChord

Don't forget to get the latest colorchord and then, on another terminal execute:

```
./colorchord shmtest.conf
```


## Pitfalls
 * You may need to generate a new OAuth key from a new google account if you get the quote overflowed issue.
 * You have to be logged in as an account that can actually chat.  There's a ton of reasons accounts can't chat.



lsof -i TCP:8888 -t | xargs -r kill -9

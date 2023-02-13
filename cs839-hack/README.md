```bash
docker build . -t ctnelson1997/cs839-hack-ui
docker push ctnelson1997/cs839-hack-ui
```

```bash
docker pull ctnelson1997/cs839-hack-ui
docker run --restart=always -d -p 39840:80 --name cs839-hack-ui ctnelson1997/cs839-hack-ui
```
# docker

### install

```shell
# in ubuntu20.04
sudo apt-get update
sudo apt-get install -y apt-transport-https ca-certificates curl software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add - 
sudo add-apt-repository "deb [arch=amd64] http://mirrors.aliyun.com/docker-ce/linux/ubuntu $(lsb_release -cs) stable"
sudo apt-get update
sudo apt-get install -y docker-ce
```



### [docker 镜像加速，修改为阿里云镜像](https://www.cnblogs.com/codeBang/p/11904924.html)

```shell
zx@u18-1:/etc/docker$ sudo cat daemon.json
{
  "registry-mirrors": ["http://hub-mirror.c.163.com"]
}

sudo systemctl daemon-reload
sudo systemctl restart docker
```




#!/bin/sh

git filter-branch --env-filter '

OLD_EMAIL="oncwnuFi0ujwfi5-C_5zg7Z0IblM@git.weixin.qq.com"
CORRECT_NAME="MQN-80"
CORRECT_EMAIL="3218290253@qq.com"

if [ "$GIT_COMMITTER_EMAIL" = "$OLD_EMAIL" ]
then
    export GIT_COMMITTER_NAME="$CORRECT_NAME"
    export GIT_COMMITTER_EMAIL="$CORRECT_EMAIL"
fi
if [ "$GIT_AUTHOR_EMAIL" = "$OLD_EMAIL" ]
then
    export GIT_AUTHOR_NAME="$CORRECT_NAME"
    export GIT_AUTHOR_EMAIL="$CORRECT_EMAIL"
fi
' --tag-name-filter cat -- --branches --tags

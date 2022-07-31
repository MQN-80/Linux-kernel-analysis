#!/bin/sh
git filter-branch -f --env-filter '

an="$梦秋年"
am="$oncwnuFi0ujwfi5-C_5zg7Z0IblM@git.weixin.qq.com"
cn="$MQN-80"
cm="$3218290253@qq.com"


oldName="old_sssss"
oldEmail="old_sssss@gmail.com"
newName="Peter"
newEmail="Peter@gmail.com"

if [ "$GIT_COMMITTER_EMAIL" = "$oldEmail" ]
then
    cn="$newName"
    cm="$newEmail"
fi

if [ "$GIT_COMMITTER_NAME" -eq "$oldName" ]
then
    cn="$newName"
    cm="$newEmail"
fi


if [ "$GIT_AUTHOR_EMAIL" = "$oldEmail" ]
then
    an="$newName"
    am="$newEmail"
fi

if [ "$GIT_AUTHOR_NAME" -eq "$oldName" ]
then
    an="$newName"
    am="$newEmail"
fi

export GIT_AUTHOR_NAME="$an"
export GIT_AUTHOR_EMAIL="$am"
export GIT_COMMITTER_NAME="$cn"
export GIT_COMMITTER_EMAIL="$cm"
'

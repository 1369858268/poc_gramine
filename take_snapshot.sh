path=etc

postfix="_replay"
path_replay=${path}${postfix}


echo "Copying the 'redis.conf' file ..."
rm -rf ${path_replay}/*
cp -r ${path}/redis.conf ${path_replay}/

echo "Step Done."

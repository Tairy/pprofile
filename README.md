# pprofile
php profile ext.

## Usage

```bash
git clone ${git-repo-add}
cd /path/to/pprofile
./configure
make && make install
cat result.json | jq -s -c 'sort_by(.wt)[]'
```

```php.ini
[pprofile]
extension=pprofile.so
pprofile.appender=1
pprofile.use_buffer=1
pprofile.env=beta
pprofile.log_dir=/log/file/prefix
```

## Reference

- [唯一ID生成原理与PHP实现](https://mp.weixin.qq.com/s/bagOgzdwLyZv_ITNVnYfoQ?)

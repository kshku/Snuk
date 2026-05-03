#!/bin/sh

for file in tests/*.snuk
do
    echo $file
    build/snuk "$file" \
    | grep -E '\[DEBUG\]: (created a ref counter|a ref counter got destroyed)' \
    | awk '
    BEGIN {
        created = 0
        destroyed = 0
    }
    {
        if ($2 == "created") created++;
        else destroyed++;
    }
    END {
        print "Created: " created
        print "Destroyed: " destroyed
    }
    '
    echo -e "\n"
done

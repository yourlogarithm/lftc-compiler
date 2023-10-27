# program de test

function max(x:int, y:int):int
    if(x<y)
        return y;
    elif (x>y)
        return x + y;
    elif (x==y)
        return 0;
    else
        return x;
    end
end

var i: int;
i = 0;    # iterator
while(i<10)
    puti(max(i,5));
    i = i + 1;
end

puts("PI=");
putr(3.14159);

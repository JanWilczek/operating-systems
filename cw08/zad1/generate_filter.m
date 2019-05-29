function [] = generate_filter(c, filename)
%generate_filter Generate random image filter and store it in file.
%   c - length of the filter in one dimension.
%   filename - path of the file to store filter's coefficients into
%   Filter's coefficients sum to one.

filter = abs(random('Stable', 2, 0, 1, 0, [c, c]));
filter = normalize(filter, 'norm', 1) ./ c;

file = fopen(filename, 'w');
fprintf(file, '%d\n', c);
for i = 1:c
    for j = 1:c
        fprintf(file, '%f ', filter(i, j));
    end
    fprintf(file, '\n');
end
fclose(file);

end

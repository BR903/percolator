curl -s -i https://api.github.com/repos/percolator/percolator/releases -H "Accept: application/vnd.github.manifold-preview+json" | grep "\"name\"\|\"download_count\""

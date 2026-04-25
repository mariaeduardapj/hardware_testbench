const catalogList = document.getElementById("catalog-list");
const boardList = document.getElementById("board-list");

function renderCatalog(catalog) {
    if (!catalogList) {
        return;
    }

    catalogList.innerHTML = catalog.deviceTypes
        .map((item) => `
            <article class="catalog-item">
                <h3>${item.label}</h3>
                <p>${item.pins === 1 ? "1 GPIO" : `${item.pins} GPIOs`}</p>
                <small>${item.pinLabels.join(" + ")}</small>
            </article>
        `)
        .join("");

    if (boardList) {
        boardList.textContent = `Tested support: ${catalog.supportedBoards.join(", ")}. ${catalog.compatibility}.`;
    }
}

async function initializeCatalogPage() {
    try {
        const response = await fetch("/catalog");
        if (!response.ok) {
            throw new Error("Unable to load catalog.");
        }

        const catalog = await response.json();
        renderCatalog(catalog);
    } catch (error) {
        if (catalogList) {
            catalogList.innerHTML = "<p>Catalog unavailable.</p>";
        }
        if (boardList) {
            boardList.textContent = error.message;
        }
    }
}

window.addEventListener("load", initializeCatalogPage);

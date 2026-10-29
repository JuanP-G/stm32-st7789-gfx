(function () {
  var root = document.documentElement;
  var key = "theme";
  try { var saved = localStorage.getItem(key); if (saved) root.setAttribute("data-theme", saved); } catch (e) {}
  var btn = document.getElementById("theme-toggle");
  if (!btn) return;
  btn.addEventListener("click", function () {
    var current = root.getAttribute("data-theme");
    if (!current) current = window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
    var next = current === "dark" ? "light" : "dark";
    root.setAttribute("data-theme", next);
    try { localStorage.setItem(key, next); } catch (e) {}
  });
})();
